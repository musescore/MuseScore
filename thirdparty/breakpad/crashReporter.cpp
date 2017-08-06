#include <iostream>
#include <fstream>

using namespace::std;

void read_metadata(string mypath){
    ifstream myfile (mypath);
    string line;

    if (myfile.is_open()){
        while ( getline (myfile,line) ){
            cout << line << '\n';
        }
        myfile.close();
    }

}


main(int argc, char *argv[]){

    string minidump_path;
    string metadata_path;

    cout << "HELLO!!!" << endl;
    if ( argc == 3 ){
        cout << argv[1] << endl;
        minidump_path = argv[1];
        metadata_path = argv[2];
        read_metadata(metadata_path);

    }
	return 0;
}

