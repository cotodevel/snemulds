#ifndef SNEMULDS_FRONTEND
#define SNEMULDS_FRONTEND



#endif



#ifdef __cplusplus
extern "C" {
#endif

size_t ucs2tombs(unsigned char* dst, const unsigned short* src, size_t len);
char* myfgets(char *buf,int n,FILE *fp);
void SplitItemFromFullPathAlias(const char *pFullPathAlias,char *pPathAlias,char *pFilenameAlias);
bool _readFrontend(char *target);
bool readFrontend(char **_name, char **_dir);

#ifdef __cplusplus
}
#endif
