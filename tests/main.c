#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>

uint64_t GetHash(uint8_t* buff, size_t length)
{
    uint64_t hash = 14695981039346656037UL;
    for (size_t i = 0; i < length; i++)
    {
        hash ^= (uint64_t)(buff[i]);
        hash *= 1099511628211UL;
    }
    return hash;
}   

int forearch_dir(const char* path, const char* base, FILE* fp)
{
    DIR* dir = opendir(path);
    if (dir == NULL)
        return 1;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        char current[512];
        sprintf(current, "%s/%s", path, entry->d_name);

        char base_dir[512];
        sprintf(base_dir, "%s/%s", (base == NULL ? "" : base), entry->d_name);

        struct stat path_stat;
        if (stat(current, &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
        {
            if (forearch_dir(current, base_dir, fp) != 0)
                return 2;
        } else
        {
            FILE* file = fopen(current, "r");
            if (file != NULL)
            {
                fseek(file, 0, SEEK_END);
                size_t length = ftell(file);
                fseek(file, 0, SEEK_SET);

                uint8_t* buffer = (uint8_t*)malloc(length + 1);
                if (fread(buffer, 1, length, file) == length)
                {
                    buffer[length] = '\0';

                    uint64_t hash = GetHash(buffer, length);
                    free(buffer);

                    fprintf(fp, "%lu %s\n", hash, base_dir);
                } else
                {
                    perror("forearch_dir");
                }

                fclose(file);
            }
        }
    }

    closedir(dir);

    return 0;
}

int main()
{
    char dir[512] = {0};
    if (getcwd(dir, sizeof(dir)) == NULL)
        return 1;

    char manifest_file[512] = {0};
    sprintf(manifest_file, "%s/bin/manifest.txt", dir);

    FILE* file = fopen(manifest_file, "w");

    char path[512];
    sprintf(path, "%s/bin/resources", dir);

    forearch_dir(path, NULL, file);

    fclose(file);
}
