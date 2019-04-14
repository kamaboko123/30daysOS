# 「30日でできる！ OS自作入門」

「[30日でできる！ OS自作入門](https://www.amazon.co.jp/dp/B00IR1HYI0)」のharibote OSをLinux環境で開発していくことを目標にしています。  
上記の書籍では、開発環境はWindowsを前提にかかれており、著作者が開発したツールなども使用されています。  
本リポジトリではGCCなどのGNU toolchain、またGNUではないが一般的に使用されているツール群を利用して開発していくつもりです。  
~~途中で挫折するかもしれません。~~

## ディレクトリ構成について
書籍の日数ごと、バージョン名ごとにディレクトリを分けています。  
各Makefileのあるディレクトリでmakeするとイメージファイルが生成されます。make runでQEMUで起動します。  
各バージョンによって差分が少なすぎる場合は、ディレクトリを作らずにスキップしているところもります。  
各バージョンのファイル名・ディレクトリ構成については、わたしの好みで作成しており、書籍に準じていません。  
また、それに合わせたMakefileを記述しています。

## ソースコードについて
基本的に写経です。  
「自分で書いて理解する」ことを目的にしているので、基本的に書籍からのコピペはしていません。  
そのため、typo等によるバグを含んでいる可能性があります。  
見つけた方はissueに書いていただくか、こっそり教えていただけると幸いです。  

## 開発環境
マシン : VM on ESXi(Core i7-8700K, RAM 8GB)  
OS : Ubuntu Server 16.04.5(amd64)  
Cコンパイラ : gcc 5.4.0  
アセンブラ : Gas(GNU assembler) 2.26.1  
リンカ : GNU ld 2.26.1  
x86エミュレータ : QEMU2.5.0  

## 参考にさせて頂いたもの（まだ開発終わってないのでこれからもっと増えます）
### 書籍

[30日でできる！ OS自作入門](https://www.amazon.co.jp/dp/B00IR1HYI0)  
この本の内容をLinux環境で簡単に使用できるツールで実装していきます。  
  
[32ビットコンピュータをやさしく語る はじめて読む486](https://www.amazon.co.jp/dp/B00OCF5YUA/ref=dp-kindle-redirect?_encoding=UTF8&btkr=1)  
486の解説書です。  
図が多く、x86アーキテクチャを初めて学ぶ私にも理解しやすかったです。  
ただ古い本であるせいか、ソースコードについては、おそらくMS-DOS？をターゲットに書かれているようで、ほとんど読み飛ばしました。  
30日OSの本は非常に良書ではあるのですが、x86の詳細な機能については解説が充実しているとは言えません。  
30日OSの本を進めている間に、割り込み, IDT, GDT, TSSなどへの理解が不足していると感じたて、この本で補強しました。  
補強のつもりで買ったのですが、想像以上に面白い本で、はりぼてOSの開発が読み終わったら、しっかり読み込みたいと思います。  
  

### ブログ・ホームページ  
[takeisa memo: OS作成](http://takeisamemo.blogspot.com/search/label/OS%E4%BD%9C%E6%88%90)  
3日目の記事にめちゃくちゃ助けられました。  
3日目はasmheadを実装していきます。  
(IPLから呼ばれて、OSイメージのメモリ上の展開や、プロテクトモードへの移行を行うプログラム。アセンブラで書く)  
まだアセンブラに全く慣れていない時期にもかかわらず、naskからGasを変換しながら書いていったため、自分一人では絶対にできなかったと思います。  
上記の記事で、nask/NASMと、Gasの差分を知ったり、リンカスクリプトの存在を知ったり、この3日目でのちに繋がっていく、結構な知識のベースを獲得できたと思います。  
  
[30日でできる!OS自作入門 まとめ | サラリーマンがハッカーを真剣に目指す](http://bttb.s1.valueserver.jp/wordpress/blog/2018/04/17/makeos/)  
各日数ごとに重要なポイントがまとまっています。  
特に参考になったのは3日目-2のところで、IPLとasmheadから、bootpack.cを呼び出すまでの道のりです。  
ここはなかなかイメージが掴めず、理解まで時間がかかったのですが、  
上記のブログの[記事](http://bttb.s1.valueserver.jp/wordpress/blog/2017/12/06/makeos-3-2/)ではメモリ上のデータ配置が図解されており、非常にわかりやすかったです。  
  
[『30日でできる！OS自作入門』のメモ](https://vanya.jp.net/os/haribote.html)  
gccで.hrb形式のバイナリを生成するための、リンカスクリプトはこちらで公開されているものを使用しました。  
  
[0から作るOS開発 「OS自作入門」](http://softwaretechnique.jp/OS_Development/index.html)  
アセンブラを書く上で、Tipsの[IA32(x86) 汎用命令一覧](http://softwaretechnique.jp/OS_Development/Tips/IA32_instructions.html)はたくさん参考にしました。  
  
[GAS_基本文法 CapmNetwork](http://capm-network.com/?tag=GAS_%E5%9F%BA%E6%9C%AC%E6%96%87%E6%B3%95)  
アセンブラ全くの初心者だったため、最初にGasの文法を知るために参考にしました。  
  
[Linux のアセンブラー: GAS と NASM を比較する - IBM](https://www.ibm.com/developerworks/jp/linux/library/l-gas-nasm.html)  
30日OSの本では、アセンブラは作者オリジナルのnaskを利用しています。  
このnaskはNASMライクなアセンブラになっています。  
今回は可能な限りGNU toolchainを利用したかったため、NASMを利用せずにGasを使用しました。  
nask/NASMはintel構文、GasはAT&T構文で、細かい違いが結構あったため上記の記事を参考にしました。  
（あとで知ったんですが、Gasでもintel構文でかけるオプションがあるらしい）  
  
[FDの構造とFAT12 物理構造と論理構造 トラック セクタ クラスタ ディレクトリ](http://park12.wakwak.com/~eslab/pcmemo/fdfat/fdfat4.html)  
30日OSの本の中に、FAT12の読み取りを簡易的に実装する箇所があります。  
そもそもFAT12の構造を理解していなかったので、このページを参考にしました。  
  
[AT&T Assembly Syntax](https://csiflabs.cs.ucdavis.edu/~ssdavis/50/att-syntax.htm)  
Gasでのfar jmp命令の書き方がよく分からず、ページ下部のNASMとの対応表を参考にしました。
  

### ツールなど
[2進数、8進数、10進数、16進数相互変換ツール - 単位変換](https://hogehoge.tk/tool/number.html)  

