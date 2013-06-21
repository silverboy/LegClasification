#include "Detector.h"
#include <mrpt/gui/CDisplayWindowPlots.h>

using namespace mrpt::gui;



int main( int argc, const char* argv[] )
{

	vector<double> ancho,profundidad,contorno;


	double ans;

	cin >> ans;

	cout << ans << endl;

	return 0;



	for(int i=10;i<20;i++){

		Detector detector;

		char nombre[100];

		sprintf(nombre,"/home/jplata/Eclipse/MedidasPiernas/laser%i.dat",i);
		cout << "Fichero:  " << nombre << endl;

		detector.abrirFichero(nombre,false);


		vector<Cluster> piernas=detector.clusterizar(0.1,3);

		detector.printClusters(piernas);

		// Obtengo parámetros

		for(int i=0;i < piernas.size();i++){
			ancho.push_back(piernas[i].getAncho());
			profundidad.push_back(piernas[i].getProfundidad());
			contorno.push_back(piernas[i].getContorno());

		}
	}

	//print_vector("%f\t",ancho);

	CDisplayWindowPlots plot1("Ancho vs contorno");
	CDisplayWindowPlots plot2("Profundidad vs contorno");


	plot1.axis(-0.1,0.5,-0.1,0.5);
	plot2.axis(-0.1,0.5,-0.1,0.5);

	plot1.plot(contorno,ancho,".3");
	plot2.plot(contorno,profundidad,".3");

	cout << "Presione cualquier tecla para terminar:" << endl;

	mrpt::system::os::getch();



}
