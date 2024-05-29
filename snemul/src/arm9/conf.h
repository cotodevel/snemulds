/*extern "C"
{*/

void set_config_file(char *filename);
void set_config_data(char *data, int length);
void override_config_file(char *filename);
void override_config_data(char *data, int length);
void flush_config_file(void);
void reload_config_texts(char *new_language);

void push_config_state(void);
void pop_config_state(void);

void hook_config_section(char *section,
			 int (*intgetter)(char *, int), 
			 char *(*stringgetter)(char *, char *), 
			 void (*stringsetter)(char *,char *));
int config_is_hooked(char *section);

char * get_config_string(char *section, char *name, char *def);
int get_config_int(char *section, char *name, int def);
int get_config_hex(char *section, char *name, int def);
int get_config_oct(char *section, char *name, int def);
float get_config_float(char *section, char *name, float def);
int get_config_id(char *section, char *name, int def);
char ** get_config_argv(char *section, char *name, int *argc);
char * get_config_text(char *msg);

void set_config_string(char *section, char *name, char *val);
void set_config_int(char *section, char *name, int val);
void set_config_hex(char *section, char *name, int val);
void set_config_oct(char *section, char *name, int size, int val);
void set_config_float(char *section, char *name, float val);
void set_config_id(char *section, char *name, int val);

int list_config_entries(char *section, char ***names);
int list_config_sections(char ***names);
void free_config_entries(char ***names);

char *find_config_section_with_hex(char *name, int hex);
char *find_config_section_with_string(char *name, char *str);
int	is_section_exists(char *section);

void save_config_file();

//}
