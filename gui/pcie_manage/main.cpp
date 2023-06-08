#include "main_window.h"
#include <QApplication>
//#include <fcntl.h>

int main(int argc, char *argv[])
{
//    int fd;
//    char dma_path[50];
//    memset (dma_path, '\0', 50);
//    sprintf (dma_path, "/dev/%s%d_%d","ps_pcie_dmachan",1,0);

//    fd = open(dma_path, O_RDWR);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
