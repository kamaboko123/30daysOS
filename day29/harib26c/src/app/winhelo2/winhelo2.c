#include "apilib.h"

//翌日(23日目)の最初でバイナリサイズを見て大きいことが課題になる
//けど、こうしないとdataセクションに確保されなくて、バイナリサイズが小さくなってしまう様子。
//初期化しないとおそらくbssセクション？に置かれて、ローダがプログラムを配置するときに確保することになる？？（なんもわからん）
//追記：
//gccのオプションに-fno-commonをつけると初期化しなくてもデータセクションに確保される
//はりぼてOSのアプリケーションは、スタックが一番低位で、bssセクションは一番高位に配置される
//なので、このbufという変数がbssセクションに置かれる場合、ローダ部分で確保したデータセグメントの最後の方を使うことになる
//データセグメントを超えない限りは問題ない、超えてもセグメントのおかげでアプリケーションが落ちるだけ
//普通にLinuxとかでローカル変数・グローバル変数確保しすぎてセグフォするのと同じ
//むしろデータセクションに置くとバイナリサイズがでかくなるから、サイズが気になるなら初期化はアプリ内でやったほうがいい
//速度が気になるならデータセクションに置くのはありだけど、起動時のコピーが増えるので、起動が遅くなる
//harib25b追記
//ローカル変数において、スタック領域に確保することにする

void HariMain(void){
    char buf[150 * 50];
    int win;
    win = api_openwin(buf, 150, 50, -1, "hello");
    api_boxfillwin(win, 8, 36, 141, 43, 3); //黄色
    api_putstrwin(win, 28, 28, 0, 12, "hello, world");
    
    for(;;) if(api_getkey(1) == 0x0a) break;
    api_end();
}
