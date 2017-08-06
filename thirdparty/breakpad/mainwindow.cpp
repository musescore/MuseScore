#include "mainwindow.h"
#include "ui_mainwindow.h"

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


void MainWindow::on_pushButton_clicked()
{

    QString minidump_path;
    QString metadata_path;



    cout << "HELLO!!!" << endl;
    if ( QCoreApplication::arguments().count() == 3 ){
        //cout << argv[1] << endl;
        minidump_path = QCoreApplication::arguments().at(1);
        metadata_path = QCoreApplication::arguments().at(2);
        cout << "Minidump path: " << minidump_path.toStdString() << endl;
        read_metadata(metadata_path.toStdString());

        map<wstring, wstring> parameters;
        map<wstring, wstring> files;
        wstring response;
        int mytimeout, my_error;

        // Add any attributes to the parameters map.
        // Attributes such as uname.sysname, uname.version, cpu.count are
        // extracted from minidump files automatically.
        parameters.insert(pair<wstring, wstring>(L"product_name", L"foo"));
        parameters.insert(pair<wstring, wstring>(L"version", L"0.1.0"));
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

        QMessageBox::information(this,"Message","Report has send!!");

    }
}

void MainWindow::on_pushButton_2_clicked()
{
    close();
}
