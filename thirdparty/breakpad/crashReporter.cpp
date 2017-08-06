#include <iostream>
#include <fstream>

#include "common/windows/http_upload.h"

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

wstring str2wstr(string mystr){
    wstring res(mystr.begin(), mystr.end());
    return res;
}

string wstr2str(wstring mystr){
    string res(mystr.begin(), mystr.end());
    return res;
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

        map<wstring, wstring> parameters;
        map<wstring, wstring> files;
        wstring response;
        int mytimeout, my_error;

        // Add any attributes to the parameters map.
        // Attributes such as uname.sysname, uname.version, cpu.count are
        // extracted from minidump files automatically.
        parameters.insert(pair<wstring, wstring>(L"product_name", L"foo"));
        parameters.insert(pair<wstring, wstring>(L"version", L"0.1.0"));
        files.insert(pair<wstring, wstring>(L"upload_file_minidump", str2wstr(minidump_path)));

        wstring url = L"https://musescore.sp.backtrace.io:6098/post?format=minidump&token=00268871877ba102d69a23a8e713fff9700acf65999b1f043ec09c5c253b9c03";

        google_breakpad::HTTPUpload *test_upload;
        test_upload->SendRequest(url,
                                   parameters,
                                   files,
                                   &mytimeout,
                                   &response,
                                   &my_error);

        cout << wstr2str(response) << endl;

    }
	return 0;
}

