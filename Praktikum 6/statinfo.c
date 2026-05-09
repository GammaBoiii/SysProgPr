#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fuer close
#include <sys/stat.h> // fuer stat
#include <fcntl.h> // fuer Dateioperationen
#include <time.h> // fuer Zeitumrechnung
#include <pwd.h> // fuer getpwuid
#include <grp.h> // fuer getgrpid

void get_permissions_string(mode_t mode, char *str);
int checkStatusInfo(const char* dateiname);


/*
int main(int argc, char const *argv[])
{
	if(argc < 2) {
		perror("Fehlender Parameter");
		exit(EXIT_FAILURE);
	}
	checkStatusInfo(argv[1]);
	return 0;
}
*/

int checkStatusInfo(const char* dateiname) {
	int fd;
	struct stat statbuf;
	if((fd = open(dateiname, O_RDONLY)) == -1) {
		perror("Datei existiert nicht\n");
	} else {
		printf("\n\nDatei %s existiert. Hier sind die folgenden Informationen:\n", dateiname);
		fstat(fd, &statbuf);
		printf("Gerätenummer: %ld\n", statbuf.st_dev);
		printf("Datei hat inode %ld\n", statbuf.st_ino);
		printf("Linkzähler: %ld\n", statbuf.st_nlink);

		if(S_ISREG(statbuf.st_mode)) {
			printf("Dateityp: reguläre Datei\n");
		} else if(S_ISDIR(statbuf.st_mode)) {
			printf("Dateityp: Verzeichnis\n");
		} else {
			printf("Dateityp: Andere\n");
		}
		
		
		
		char perms [12];
		get_permissions_string(statbuf.st_mode, perms);
		printf("Zugriffsrechte (u-g-o): %s \n", perms);

		struct passwd *pw = getpwuid(statbuf.st_uid);
		struct group *gr = getgrgid(statbuf.st_gid);
		printf("Beistzer - %s & Gruppe - %s + UID(%d)/GUID(%d): \n", pw->pw_name, gr->gr_name, pw->pw_uid, gr->gr_gid);
		printf("Dateigröße: %ld Bytes\n", statbuf.st_size);

		struct tm *tm_info = localtime(&statbuf.st_mtime);
		printf("Letzte Änderung: %02d.%02d.%04d %02d:%02d:%02d\n", (*tm_info).tm_mday + 1, tm_info->tm_mon + 1, tm_info->tm_year + 1900, tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);

		close(fd);
	}
}

void get_permissions_string(mode_t mode, char *str) {
    //user
    str[0] = (mode & S_IRUSR) ? 'r' : '-';
    str[1] = (mode & S_IWUSR) ? 'w' : '-';
    str[2] = (mode & S_IXUSR) ? 'x' : '-';
	str[3] = '/';
    //group
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
	str[7] = '/';
    //other
    str[8] = (mode & S_IROTH) ? 'r' : '-';
    str[9] = (mode & S_IWOTH) ? 'w' : '-';
    str[10] = (mode & S_IXOTH) ? 'x' : '-';
    str[11] = '\0';
}
