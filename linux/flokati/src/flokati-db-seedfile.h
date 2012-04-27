
#include <dirent.h>
#include <sys/types.h>
#include <regex.h>

struct dbent {
	struct dbent *next;
	uint64_t family;
	uint64_t mask;
	char *content;
	long size;
};
// just store devices as a list for now
struct dbent *devices=NULL;

void db_init( const char *jsondir ) {
	DIR *dir;
	struct dirent *ent;
	
	FILE *fp;
	char *filecontent;
	long filesize;
	
	struct json_object *definition, *family_js, *mask_js;
	char *family_s, *family_s2;
	uint64_t family;
	int mask;
	
	regex_t filter;
	regcomp(&filter, "\\.json$", 0);
	
	dir = opendir(jsondir);
	if (dir) {
		chdir(jsondir);
		while ((ent = readdir(dir)) != NULL) {
			if (regexec(&filter, ent->d_name, 0, NULL, 0) == 0) {
				
				printf("Found file %s... ",ent->d_name);
				
				fp = fopen(ent->d_name, "r");
				if (!fp) {
					perror("Cannot open file");
					continue;
				}
				fseek(fp, 0L, SEEK_END);
				filesize = ftell(fp);
				fseek(fp, 0L, SEEK_SET);
				
				filecontent = malloc(sizeof(char)*filesize+1);
				filesize = fread(filecontent, sizeof(char), filesize+1, fp);
				fclose(fp);
				
				definition = json_tokener_parse(filecontent);
				family_js = json_object_object_get(definition, "family");
				family_s = (char *)json_object_get_string(family_js);
				mask_js = json_object_object_get(definition, "mask");
				mask = json_object_get_int(mask_js);
				
				if (family_s)
					family = (uint64_t)strtoull(family_s, &family_s2, 16);
					//strtoull seems to be necessary for mips?
				
				printf("f.s: %s, f.i: %08x%08x, m: %i\n",family_s, (unsigned int)(family>>32), (unsigned int)(family), mask);
				
				if (family_s && mask>0 && mask<64 && (family_s<family_s2) && ((family_s2-16) <= family_s) 
					&& family>0 && family<((uint64_t)-1) ) {
					
					struct dbent *newent = malloc(sizeof(struct dbent));
					newent->next=NULL;
					newent->family=family;
					newent->mask=0;
					newent->content=filecontent;
					newent->size=filesize;
					
					uint8_t i;
					for(i=1; i<=64; i++) {
						newent->mask <<= 1;
						if (i<=mask) newent->mask++;
					}
					
					struct dbent *d = devices;
					if (d) {
						while(d->next) d=d->next;
						d->next=newent;
					} else {
						devices=newent;
					}
					printf("successfully added: %08x%08x/%i\n",(unsigned int)(family>>32),(unsigned int)(family),mask);
				} else {
					free(filecontent);
					printf("JSON-content invalid\n");
				}
				json_object_put(definition);
			} else {
// 				printf("%s doesn't match pattern.\n",ent->d_name);
			}
		}
		closedir(dir);
		printf("Device DB initialized from files.\n");
	} else {
		perror("Cannot open directory.");
	}
	regfree(&filter);
}

long db_get(uint64_t id, char **ptr) {
	struct dbent *d=devices;
	while (d) {
		if ((id & d->mask) == d->family) {
			*ptr=d->content;
			return (d->size);
		} else {
			d=d->next;
		}
	}
	return -1;
}