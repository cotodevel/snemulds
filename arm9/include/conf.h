#ifndef conf_snemulds
#define conf_snemulds

#if 0
typedef struct CONFIG_ENTRY
{
   char *name;                      // variable name (NULL if comment) 
   char *data;                      // variable value
   struct CONFIG_ENTRY *next;       // linked list
} CONFIG_ENTRY;

typedef struct CONFIG_SECTION
{
   char *name;                      // variable name (NULL if comment) 
   char *data;                      // variable value 
   int	*key;			    // one or more keys

   struct CONFIG_SECTION *next;       // linked list 
   struct CONFIG_ENTRY	*head;	    // linked list 
} CONFIG_SECTION;
#else
typedef struct CONFIG_ENTRY
{
   char *name;                      // variable name (NULL if comment) 
   char *data;                      // variable value 
   struct CONFIG_ENTRY *next;       // linked list 
} CONFIG_ENTRY;
#endif


typedef struct CONFIG
{
#if 0
   CONFIG_SECTION *head;              //linked list of config entries 
#else
   CONFIG_ENTRY *head;              // linked list of config entries 
#endif
   char *filename;                  // where were we loaded from? 
   int dirty;                       // has our data changed? 
} CONFIG;


typedef struct CONFIG_HOOK
{
   char *section;                   // hooked config section info 
   int (*intgetter)(char *name, int def);
   char *(*stringgetter)(char *name, char *def);
   void (*stringsetter)(char *name, char *value);
   struct CONFIG_HOOK *next; 
} CONFIG_HOOK;

#endif

#ifdef __cplusplus
extern "C" {
#endif

void set_config_file(char *filename);
void set_config_data(char *data, int length);
void override_config_file(char *filename);
void override_config_data(char *data, int length);
void flush_config_file(void);
void reload_config_texts(char *new_language);

void push_config_state(void);
void pop_config_state(void);

void hook_config_section(char *section,int (*intgetter)(char *, int), char *(*stringgetter)(char *, char *), 
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


extern void save_config_file();
extern void save_config(CONFIG *cfg);

extern void destroy_config(CONFIG *cfg);
extern void init_config(int loaddata);

extern void set_config(CONFIG **config, char *data, int length, char *filename);
extern void load_config_file(CONFIG **config, char *filename, char *savefile);
extern void prettify_section_name(char *in, char *out);
extern CONFIG_ENTRY *find_config_string(CONFIG *config, char *section, char *name, CONFIG_ENTRY **prev);
extern CONFIG_ENTRY *insert_variable(CONFIG *the_config, CONFIG_ENTRY *p, char *name, char *data);
extern int get_line(char *data, int length, char *name, char *val);

//gui
void		GUI_printf(char *fmt, ...);

#ifdef __cplusplus
}
#endif
