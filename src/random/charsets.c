int isadigit(char c){
    int rv = 0;
    getcharsets();
    for(size_t i = 0; i < sizeof(buf_digits) && !rv; i++)
        rv = buf_digits[i] == c;
    return rv;
}

static const u_int pos_lowercase[2] = {97, 122},
                   pos_uppercase[2] = {65, 91},
                   pos_digits[2] = {48, 58};

#define getcharset(BUF,SRC) do { \
    const u_int pos_start = SRC[0], pos_end = SRC[1]; \
    u_int pos = 0; \
    for(u_int i = pos_start; i < pos_end || pos < sizeof(BUF) - 1; i++) \
        BUF[pos++] = i; \
} while(0)

void get_lower_charset(void){
    if(buf_lowercase[0] != 0) return;
    getcharset(buf_lowercase, pos_lowercase);
}

void get_upper_charset(void){
    if(buf_uppercase[0] != 0) return;
    getcharset(buf_uppercase, pos_uppercase);
}

void get_digits_charset(void){
    if(buf_digits[0] != 0) return;
    getcharset(buf_digits, pos_digits);
}

void getcharsets(void){
    if(got_charsets) return;
    get_lower_charset();
    get_upper_charset();
    get_digits_charset();
    got_charsets = 1;
}