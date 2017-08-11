#include "mainwindow.h"
#include "ui_mainwindow.h"

wstring str2wstr(string mystr){
    wstring res(mystr.begin(), mystr.end());
    return res;
}

string wstr2str(wstring mystr){
    string res(mystr.begin(), mystr.end());
    return res;
}

pair<wstring,wstring> line2strings(string line){
        string str1, str2;
        string *mystr;
        bool myflag;
        int i;

        myflag = false;
        str1.empty();
        str2.empty();

        mystr = &str1;

        for (i = 0; i < line.size(); i++){
                if ( line[i] == ',' && myflag == false){
                        mystr = &str2;
                        myflag = true;
                }
                else{
                        if (line[i] != '\n'){
                                *mystr += line[i];
                        }

                }
        }

        return pair<wstring,wstring>(str2wstr(str1),str2wstr(str2));

}


map <wstring,wstring> read_csv(string mypath){
        string line;
        pair<wstring,wstring> mykeyval;
        ifstream myfile(mypath);
        map <wstring,wstring> mymap;

        if (myfile.is_open()){
                while ( getline (myfile,line) ){
                        cout << line << '\n';
                        mykeyval = line2strings(line);
                        //cout << "str1: " << mykeyval.first << " str2: " << mykeyval.second << endl;
                        mymap.insert(mykeyval);

                }
                myfile.close();

        }
        else{
            cout << "Unable to open file";
        }

        return mymap;
}

void sendReport(QString user_txt){
    QString minidump_path;
    QString metadata_path;
    map<wstring, wstring> parameters;
    map<wstring, wstring> files;

    if ( QCoreApplication::arguments().count() == 3 ){
        //cout << argv[1] << endl;
        minidump_path = QCoreApplication::arguments().at(1);
        metadata_path = QCoreApplication::arguments().at(2);
        cout << "Minidump path: " << minidump_path.toStdString() << endl;
        parameters = read_csv(metadata_path.toStdString());
        parameters.insert(pair<wstring, wstring>(L"user_crash_input", user_txt.toStdWString()));

        wstring response;
        int mytimeout, my_error;

        // Add any attributes to the parameters map.
        // Attributes such as uname.sysname, uname.version, cpu.count are
        // extracted from minidump files automatically.
        //parameters.insert(pair<wstring, wstring>(L"product_name", L"foo"));
        //parameters.insert(pair<wstring, wstring>(L"version", L"0.1.0"));
        files.insert(pair<wstring, wstring>(L"upload_file_minidump", minidump_path.toStdWString()));

        wstring url = L"https://musescore.sp.backtrace.io:6098/post?format=minidump&token=00268871877ba102d69a23a8e713fff9700acf65999b1f043ec09c5c253b9c03";

        google_breakpad::HTTPUpload *test_upload;
        test_upload->SendRequest(url,
                                   parameters,
                                   files,
                                   &mytimeout,
                                   &response,
                                   &my_error);

        cout << wstr2str(response) << endl;

        //QMessageBox::information(this,"Message","Report has send!!");

    }
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked(){

    if ( ui->checkBox->isChecked() ){
        QString user_txt = ui->plainTextEdit->toPlainText();
        sendReport(user_txt);
    }

    close();

}

void MainWindow::on_pushButton_2_clicked(){

    if ( ui->checkBox->isChecked() ){
        QString user_txt = ui->plainTextEdit->toPlainText();
        sendReport(user_txt);
    }

    // TODO: restart MuseScore

}
