#include <bits/stdc++.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;


// void create_file() {
// 	FILE *file;
// 	file = fopen ("test_file.txt","w");
// 	if (file!=NULL) {
//     	fprintf(file,"Novo arquivo");
//     	fclose (file);
//  	} else {
//   		printf("Arquivo não encontrado\n");
//   	}
//   	printf("Arquivo salvo\n");
// }

void test_file() {
	FILE *file;
	file = fopen("test_file.txt","r");
	if (file != NULL) {
		printf("File found!\n");
		fclose (file);
  	} else {
  		printf("File not found :(\n");
  	}
}

void pid_test() {
	pid_t pid = getpid();
	printf("My pid is %d\n",pid);
}

void env_variable() {
	char env_var_name[] = "HOME";
	char* var = getenv(env_var_name);
	if (var != NULL)
		printf("Variable %s found : %s\n",env_var_name,var);
	else
		printf("Variable not found :(\n");
}

void test_uts() {
	const size_t max_len = 100;
	char nome[max_len];
	gethostname(nome, max_len);
	printf("[Hostname]=%s\n", nome);
}

int main() {
	test_file();
	pid_test();
	env_variable();
	test_uts();
	return 0;
}
