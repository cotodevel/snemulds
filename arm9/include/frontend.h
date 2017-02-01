#ifndef SNEMULDS_FRONTEND
#define SNEMULDS_FRONTEND

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern size_t ucs2tombs(unsigned char* dst, const unsigned short* src, size_t len);
extern char* myfgets(char *buf,int n,FILE *fp);
extern void SplitItemFromFullPathAlias(const char *pFullPathAlias,char *pPathAlias,char *pFilenameAlias);
extern bool _readFrontend(char *target);
extern bool readFrontend(char **_name, char **_dir);

#ifdef __cplusplus
}
#endif
