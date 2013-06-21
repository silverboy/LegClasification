/*
 * clasificar.cpp


 *
 *  Created on: 12/06/2013
 *      Author: jplata
 */

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "svm.h"
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

void unsort_data(char *input_file, char *output_file);
void parse_command_line(int argc, char **argv, char *input_file_name, char *model_file_name);
void read_problem(const char *filename);
void do_cross_validation();

struct svm_parameter param;		// set by parse_command_line
struct svm_problem prob;		// set by read_problem
struct svm_model *model;
struct svm_node *x_space;


static char *line = NULL;
static int max_line_len;



using namespace std;

struct Cluster{
	int clase;
	double contorno;
	double ancho;
	double profundidad;

};


int main( int argc, const char* argv[] )
{

	if(argc < 2 || argc >3){
		cout << "Uso: Clasificar archivo_datos [r]" << endl;
		cout << "El tercer parametro es opcional, si está presenta se reordenaran los datos aleatoriamente" << endl;
		return -1;
	}

	char input_file_name[1024];
	char model_file_name[]="svm_model";
	const char *error_msg;





	strcpy(input_file_name,argv[1]);

	if(argc == 3 && argv[2][0]=='r'){
		// Reordenar archivo
		cout << "Reordenar" << endl;
		FILE* svm_data=fopen(input_file_name,"r");

		if(!svm_data){
			cout << "Imposible leer archivo datos: " << input_file_name << endl;
			return -1;
		}
		else{
			string output_file="unsorted_";
			output_file.append(input_file_name);
			unsort_data(input_file_name,output_file.data());
			strcpy(input_file_name,output_file.data());
		}
	}


	// Configurar SVM
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = 4;
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 2048;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;


	void (*print_func)(const char*) = NULL;	// default printing to stdout


	svm_set_print_string_function(print_func);

	// Read problem
	read_problem(input_file_name);

	error_msg = svm_check_parameter(&prob,&param);

	if(error_msg)
	{
		fprintf(stderr,"ERROR: %s\n",error_msg);
		exit(1);
	}

	// Realizamos validacion cruzada
	do_cross_validation();

	// Construimos la SVM
	model=svm_train(&prob,&param);

	// Veamos la precision de la máquina generada
	int i;
	int vp(0),vn(0),fp(0),fn(0);
	int total_correct = 0;
	double target;


	for(i=0;i<prob.l;i++){
		target=svm_predict(model,prob.x[i]);


		if(target == prob.y[i]){
			++total_correct;
			// Acierto, determinar si positivo o negativo
			if(target == 1){
				vp++;
			}
			else{
				vn++;
			}
		}
		else{
			// Fallo, determinar si falso positivo o falso negativo
			if(target == 1){
				fp++;
			}
			else{
				fn++;
			}
		}
	}
	printf("Cross Validation Accuracy = %g%%\n",100.0*total_correct/prob.l);
	printf("\n\n");
	printf("					Valor Real\n");
	printf("\n");
	printf("				Positivo	Negativo\n");
	printf("\n");
	printf("	      Positivo          %2.2f            %2.2f\n",(double)vp*100/(vp+fn),(double)fp*100/(fp+vn));
	printf("Prediccion\n");
	printf("	      Negativo          %2.2f            %2.2f\n",(double)fn*100/(vp+fn),(double)vn*100/(fp+vn));

	// Guardamos el modelo

	if(svm_save_model(model_file_name,model)){
		printf("No pudo salvarse el modelo\n");
	}
	else{
		printf("Modelo salvado correctamente en archivo svm_model\n");
	}


}

void unsort_data(char *input_file, char *output_file){

	// Leemos cluster del archivo
	FILE* svm_file=fopen(input_file,"r");


	// Fichero para guardar datos para la SVM
	FILE* svm_file_unsorted=fopen(output_file,"w");

	vector<Cluster> conjuntos;

	Cluster grupo;

	double c,a,p;
	int clase;


	while(!feof(svm_file)){
		fscanf(svm_file,"%d 1:%lf 2:%lf 3:%lf\n",&clase, &c, &a, &p);

		grupo.clase=clase;
		grupo.ancho=a;
		grupo.profundidad=p;
		grupo.contorno=c;

		conjuntos.push_back(grupo);

	}

	int elemento;

	while(!conjuntos.empty()){
		cout << "Tamaño vector " << conjuntos.size() << endl;
		// Mientres queden cluster selecciono un indice al azar
		elemento=rand() % conjuntos.size();

		// Almaceno el elmento seleccionado
		fprintf(svm_file_unsorted,"%d 1:%f 2:%f 3:%f\n",conjuntos[elemento].clase, conjuntos[elemento].contorno,
				conjuntos[elemento].ancho,conjuntos[elemento].profundidad);
		conjuntos.erase(conjuntos.begin()+elemento);
	}

	fclose(svm_file);
	fclose(svm_file_unsorted);
}



void exit_input_error(int line_num)
{
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}



static char* readline(FILE *input)
{
	int len;

	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

int do_process(int argc, char **argv)
{
	char input_file_name[1024];
	char model_file_name[1024];
	const char *error_msg;

	//parse_command_line(argc, argv, input_file_name, model_file_name);
	read_problem(input_file_name);
	error_msg = svm_check_parameter(&prob,&param);

	if(error_msg)
	{
		fprintf(stderr,"ERROR: %s\n",error_msg);
		exit(1);
	}

	if(true)
	{
		do_cross_validation();
	}
	else
	{
		model = svm_train(&prob,&param);
		if(svm_save_model(model_file_name,model))
		{
			fprintf(stderr, "can't save model to file %s\n", model_file_name);
			exit(1);
		}
		svm_free_and_destroy_model(&model);
	}
	svm_destroy_param(&param);
	free(prob.y);
	free(prob.x);
	free(x_space);
	free(line);

	return 0;
}

void do_cross_validation()
{
	int i;
	int vp(0),vn(0),fp(0),fn(0);
	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *target = Malloc(double,prob.l);

	svm_cross_validation(&prob,&param,10,target);
	if(param.svm_type == EPSILON_SVR ||
	   param.svm_type == NU_SVR)
	{
		for(i=0;i<prob.l;i++)
		{
			double y = prob.y[i];
			double v = target[i];
			total_error += (v-y)*(v-y);
			sumv += v;
			sumy += y;
			sumvv += v*v;
			sumyy += y*y;
			sumvy += v*y;
		}
		printf("Cross Validation Mean squared error = %g\n",total_error/prob.l);
		printf("Cross Validation Squared correlation coefficient = %g\n",
			((prob.l*sumvy-sumv*sumy)*(prob.l*sumvy-sumv*sumy))/
			((prob.l*sumvv-sumv*sumv)*(prob.l*sumyy-sumy*sumy))
			);
	}
	else
	{
		for(i=0;i<prob.l;i++){
			if(target[i] == prob.y[i]){
				++total_correct;
				// Acierto, determinar si positivo o negativo
				if(target[i]==1){
					vp++;
				}
				else{
					vn++;
				}
			}
			else{
				// Fallo, determinar si falso positivo o falso negativo
				if(target[i]==1){
					fp++;
				}
				else{
					fn++;
				}
			}
		}
		printf("Cross Validation Accuracy = %g%%\n",100.0*total_correct/prob.l);
		printf("\n\n");
		printf("					Valor Real\n");
		printf("\n");
		printf("				Positivo	Negativo\n");
		printf("\n");
		printf("	      Positivo          %2.2f            %2.2f\n",(double)vp*100/(vp+fn),(double)fp*100/(fp+vn));
		printf("Prediccion\n");
		printf("	      Negativo          %2.2f            %2.2f\n",(double)fn*100/(vp+fn),(double)vn*100/(fp+vn));

	}
	free(target);
}


// read in a problem (in svmlight format)

void read_problem(const char *filename)
{
	int elements, max_index, inst_max_index, i, j;
	FILE *fp = fopen(filename,"r");
	char *endptr;
	char *idx, *val, *label;

	if(fp == NULL)
	{
		fprintf(stderr,"can't open input file %s\n",filename);
		exit(1);
	}

	prob.l = 0;
	elements = 0;

	max_line_len = 1024;
	line = Malloc(char,max_line_len);
	while(readline(fp)!=NULL)
	{
		char *p = strtok(line," \t"); // label

		// features
		while(1)
		{
			p = strtok(NULL," \t");
			if(p == NULL || *p == '\n') // check '\n' as ' ' may be after the last feature
				break;
			++elements;
		}
		++elements;
		++prob.l;
	}
	rewind(fp);

	prob.y = Malloc(double,prob.l);
	prob.x = Malloc(struct svm_node *,prob.l);
	x_space = Malloc(struct svm_node,elements);

	max_index = 0;
	j=0;
	for(i=0;i<prob.l;i++)
	{
		inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
		readline(fp);
		prob.x[i] = &x_space[j];
		label = strtok(line," \t\n");
		if(label == NULL) // empty line
			exit_input_error(i+1);

		prob.y[i] = strtod(label,&endptr);
		if(endptr == label || *endptr != '\0')
			exit_input_error(i+1);

		while(1)
		{
			idx = strtok(NULL,":");
			val = strtok(NULL," \t");

			if(val == NULL)
				break;

			errno = 0;
			x_space[j].index = (int) strtol(idx,&endptr,10);
			if(endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
				exit_input_error(i+1);
			else
				inst_max_index = x_space[j].index;

			errno = 0;
			x_space[j].value = strtod(val,&endptr);
			if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(i+1);

			++j;
		}

		if(inst_max_index > max_index)
			max_index = inst_max_index;
		x_space[j++].index = -1;
	}

	if(param.gamma == 0 && max_index > 0)
		param.gamma = 1.0/max_index;

	if(param.kernel_type == PRECOMPUTED)
		for(i=0;i<prob.l;i++)
		{
			if (prob.x[i][0].index != 0)
			{
				fprintf(stderr,"Wrong input format: first column must be 0:sample_serial_number\n");
				exit(1);
			}
			if ((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index)
			{
				fprintf(stderr,"Wrong input format: sample_serial_number out of range\n");
				exit(1);
			}
		}

	fclose(fp);
}




