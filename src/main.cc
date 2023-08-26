#include "bcsv.h"
#include "kmp.h"
#include "trie.h"
#include <netinet/in.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/tcp.h>
#include <sys/mman.h>
#include <sys/stat.h>

static id_t
nfq_get_packet_id(struct nfq_data* tb)
{
        id_t                         id = 0;
        struct nfqnl_msg_packet_hdr* ph;
        ph = nfq_get_msg_packet_hdr(tb);
        if (ph) id = ntohl(ph->packet_id);
        return id;
}

static int
callback(struct nfq_q_handle* qh, struct nfgenmsg*, struct nfq_data* nfa, void* rtrie)
{
        unsigned char* data;
        uint32_t       st = NF_ACCEPT;
        ;
        ssize_t   r  = nfq_get_payload(nfa, &data);
        u_int32_t id = nfq_get_packet_id(nfa);
        
        TRIE* trie = reinterpret_cast<TRIE*>(rtrie);

        for (; r != -1;)
        {
                size_t ofs = 0;

                iphdr  ip  = *reinterpret_cast<iphdr*>(data + ofs);
                ofs += sizeof(ip);
                if (ip.protocol != IPPROTO_TCP) break;

                tcphdr tcp = *reinterpret_cast<tcphdr*>(data + ofs);
                ofs += sizeof(tcp);
                if (ntohs(tcp.source) != 80 && ntohs(tcp.dest) != 80) break;

                ArraySegment body{
                    reinterpret_cast<const char*>(data + ofs + tcp.doff + 4),
                    r - ofs
                };

                constexpr auto hlit = "Host: ";
                constexpr auto hlen = __builtin_strlen(hlit);
                ssize_t        pos  = KMP::First(body.Ptr, body.Size, hlit, hlen);
                if (pos == -1) break;

                auto* p = body.Ptr + pos + hlen;
                while (isspace(*p)) p++;

                size_t size = 1;
                while (!isspace(p[size])) size++;

                if (!trie->Contains(p, size)) break;
                
                
                
                st = NF_DROP;
#ifdef DEBUG
                write(1, p, size);
                std::cout << " droped\n";
#endif
                break;
        }

        return nfq_set_verdict(qh, id, st, 0, nullptr);
}

void
loop(TRIE* trie)
{
        struct nfq_handle*   h;
        struct nfq_q_handle* qh;
        int                  fd;
        ssize_t              rv;
        char                 buf[4096] __attribute__((aligned));

        printf("opening library handle\n");
        h = nfq_open();
        if (!h)
        {
                fprintf(stderr, "error during nfq_open()\n");
                exit(1);
        }

        printf("unbinding existing nf_queue handler for AF_INET (ifany)\n ");
        if (nfq_unbind_pf(h, AF_INET) < 0)
        {
                fprintf(stderr, "error during nfq_unbind_pf()\n");
                exit(1);
        }

        printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
        if (nfq_bind_pf(h, AF_INET) < 0)
        {
                fprintf(stderr, "error during nfq_bind_pf()\n");
                exit(1);
        }

        printf("binding this socket to queue '0'\n");
        qh = nfq_create_queue(h, 0, &callback, trie);
        if (!qh)
        {
                fprintf(stderr, "error during nfq_create_queue()\n");
                exit(1);
        }

        printf("setting copy_packet mode\n");
        if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0)
        {
                fprintf(stderr, "can't set packet_copy mode\n");
                exit(1);
        }

        fd = nfq_fd(h);

        for (;;)
        {
                rv = recv(fd, buf, sizeof(buf), 0);
                if (rv >= 0)
                        nfq_handle_packet(h, buf, static_cast<int>(rv));
                else if (errno == ENOBUFS)
                        printf("losing packets!\n");
                else
                        break;
        }

        printf("unbinding from queue 0\n");
        nfq_destroy_queue(qh);

        printf("closing library handle\n");
        nfq_close(h);
}

int
main(int argc, char* argv[])
{
        if (argc < 2)
        {
                fprintf(stderr, "error: required argument missing\n"
                                "note: netfilter-test <csv>\n"
                                "sample: netfilter-test test.csv");
                return -1;
        }
        else if (argc > 2)
        {
                fprintf(stderr,
                        "warn: unrecognized argument will be ignored\n"
                        "note: netfilter-test %s %s\n"
                        "                       ^ here\n",
                        argv[1], argv[2]);
        }

        struct stat st;
        void*       cat;
        int         fd = open(argv[1], O_RDONLY);

        if (fd == -1)
        {
                int errsv = errno;
                perror("could not open file");
                return errsv;
        }
        fstat(fd, &st);
        cat      = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

        auto set = DataSet::Load(static_cast<const char*>(cat), st.st_size);
        if (!set)
        {
                perror("could not load data");
                return -1;
        }
        auto trie = TRIE::Compile(*set);

        munmap(cat, st.st_size);
        close(fd);

        loop(trie.get());

        return 0;
}
