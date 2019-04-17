#include "haribote.h"

//翌日(23日目)の最初でバイナリサイズを見て大きいことが課題になる
//けど、こうしないとdataセクションに確保されなくて、バイナリサイズが小さくなってしまう様子。
//初期化しないとおそらくbssセクション？に置かれて、ローダがプログラムを配置するときに確保することになる？？（なんもわからん）
char buf[150 * 50] = {0};

void HariMain(void){
    int win;
    win = api_openwin(buf, 150, 50, -1, "hello");
    api_boxfillwin(win, 8, 36, 141, 43, 3); //黄色
    api_putstrwin(win, 28, 28, 0, 12, "hello, world");
    api_end();
}
