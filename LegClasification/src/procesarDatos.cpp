#include "Detector.h"
#include <mrpt/gui/CDisplayWindowPlots.h>

using namespace mrpt::gui;

CPose2D getPataMesa();



int main( int argc, const char* argv[] )
{

	vector<double> mx,my,px,py;

	CDisplayWindowPlots medidasPlot("Medidas");
	CDisplayWindowPlots piernasPlot("Cluster");
	vector<CPose2D>* puntos;

	CPose2D pataMesa=getPataMesa();

	// Abrir archivo con perfiles a descartar
	// Dicho archivo contiene por líneas los número de los perfil a descartar
	FILE* file=fopen("descartados.dat","r");
	vector<int> ignorados;

	// Fichero para guardar datos pasa la SVM
	FILE* svm_file=fopen("svm_data.dat","w");

	if(file){
		// Archivo existe
		int indice;
		while(!feof(file)){
			fscanf(file,"%i\n",&indice);
			ignorados.push_back(indice);
		}
		fclose(file);
	}


	for(int i=0;i<552;i++){

		// Comprobar que el archivo no se encuentre entre los ignorados
		if(binary_search(ignorados.begin(),ignorados.end(),i)){
			continue;
		}

		// Salto los correspondientes al maletin
		if(i >= 377 && i <410){
			continue;
		}

		Detector detector;

		char nombre[100];

		sprintf(nombre,"/home/jplata/Eclipse/MedidasPiernas/23Mayo/laser%i.dat",i);
		cout << "Fichero:  " << nombre << endl;

		// Comprobar existencia del archivo
		FILE* file=fopen(nombre,"r");

		if(!file){
			cout << "¡¡¡¡Archivo no encontrado!!! Continuar con el siguiente" << endl;
			continue;
		}



		detector.abrirFichero(nombre,false);

		// Medidas
		puntos=detector.getPuntos();
		vector<Cluster> piernas;

		if(i > 506){
		// Clusteres detectados
			if(i > 523){
				detector.filtrarDatos();
			}
			piernas=detector.clusterizar(0.4,3);
		}
		else{
			piernas=detector.clusterizar(0.1,3);
		}



		mx.clear();
		my.clear();




		for(unsigned int i=0;i<puntos->size();i++){
			mx.push_back(puntos->at(i).x());
			my.push_back(puntos->at(i).y());
		}

		piernasPlot.clear();
		string fileName(nombre);
		piernasPlot.setWindowTitle("Cluster - " + fileName.substr(fileName.find_last_of("/")+1));
		piernasPlot.hold_on();


		// Obtengo puntos clusters
		string formato[2];
		formato[0]=".r2";
		formato[1]=".b2";

		for(int j=0;j < piernas.size();j++){

			// Si algun cluster coincide con la pata de la mesa lo elimino
			if(pataMesa.distanceTo(piernas[j].getCentro()) < 0.1){
				piernas.erase(piernas.begin()+j);
				j--;

				continue;
			}

			// Guardar datos para la SVM
			if( i<409 ){
				fprintf(svm_file,"%d 1:%f 2:%f 3:%f\n",1, piernas[j].getContorno(), piernas[j].getAncho(), piernas[j].getProfundidad());
			}
			else{
				fprintf(svm_file,"%d 1:%f 2:%f 3:%f\n",-1, piernas[j].getContorno(), piernas[j].getAncho(), piernas[j].getProfundidad());
			}


			puntos=piernas[j].getPuntos();
			px.clear();
			py.clear();

			for(unsigned int k=0;k<puntos->size();k++){
				px.push_back(puntos->at(k).x());
				py.push_back(puntos->at(k).y());
			}
			piernasPlot.plot(px,py,formato[j%2]);
		}

		/*detector.printClusters(piernas);

		medidasPlot.setWindowTitle("Medidas - " + fileName.substr(fileName.find_last_of("/")+1));
		medidasPlot.plot(mx,my,".b2");
		//piernasPlot.plot(px,py,".r2");

		medidasPlot.axis(-0.1,3,-3,3);
		piernasPlot.axis(-0.1,3,-3,3);

		cout << "Presione cualquier tecla para pasar a la siguiente muestra" << endl;

		mrpt::system::os::getch();
*/


	}

	fclose(svm_file);

	//print_vector("%f\t",ancho);



	//plot1.axis(-0.1,0.5,-0.1,0.5);
	//plot2.axis(-0.1,0.5,-0.1,0.5);


	cout << "Presione cualquier tecla para terminar:" << endl;

	mrpt::system::os::getch();



}


CPose2D getPataMesa(){

	FILE* file=fopen("patamesa.dat","r");
	CPose2D pataMesa;
	int nDatos(0);

	if(file){
		float x,y;
		// Archivo existe
		fscanf(file,"x:%f,y:%f",&x,&y);
		pataMesa.x(x);
		pataMesa.y(y);

		fclose(file);
	}
	else{
		//Archivo no existe
		file=fopen("patamesa.dat","w");

		for(int i=1;i<21;i++){

			Detector detector;

			char nombre[100];

			sprintf(nombre,"/home/jplata/Eclipse/MedidasPiernas/23Mayo/laser%i.dat",i);
			cout << "Fichero:  " << nombre << endl;

			detector.abrirFichero(nombre,false);

			// Clusteres detectados
			vector<Cluster> piernas=detector.clusterizar(0.1,3);

			detector.printClusters(piernas);

			// El ultimo cluster corresponde a la pata de la mesa en las primeras muestras
			nDatos++;

			pataMesa+=piernas.back().getCentro();

		}

		pataMesa.x(pataMesa.x()/nDatos);
		pataMesa.y(pataMesa.y()/nDatos);

		fprintf(file,"x:%f,y:%f",pataMesa.x(),pataMesa.y());

		fclose(file);

	}

	return pataMesa;

}
