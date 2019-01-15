#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 16
#define OUT_LEN 0xff

#define W 8
#define H 16

#define ASM_LABEL "hankaku"

struct ascii {
    unsigned char line[16];
};

void convert(FILE *fp, struct ascii *out);

int main(int argc, char *argv[]){
    FILE *fp;
    char buf[BUF_LEN] = {0};
    
    struct ascii *out = malloc(sizeof(struct ascii) * OUT_LEN); //エラー処理省略
    
    //引数チェック
    if(argc != 2){
        fprintf(stderr, "Args error.\nYou must specify ascii font file.\n");
        exit(-1);
    }
    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "failed to open file : %s.\n", argv[1]);
        exit(-1);
    }
    
    //初期化
    for(int i = 0; i < OUT_LEN; i++){
        for(int j = 0; j < H; j++){
            out[i].line[j] = 0;
        }
    }
    
    //カウンタ
    int index = 0;
    while(fgets(buf, BUF_LEN, fp) != NULL){
        //charを見つけたら次の行から字形
        if(strstr(buf, "char") == NULL) continue;
        
        //変換
        convert(fp, (out + index));
        index++;
    }
    
    fclose(fp);
    
    //出力
    
    printf(".data\n");
    printf(".global %s\n\n", ASM_LABEL);
    printf("%s:\n", ASM_LABEL);
    for(int i = 0; i < index; i++){
        printf("    .byte 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", out[i].line[0], out[i].line[1], out[i].line[2], out[i].line[3]);
        printf("    .byte 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", out[i].line[4], out[i].line[5], out[i].line[6], out[i].line[7]);
        printf("    .byte 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", out[i].line[8], out[i].line[9], out[i].line[10], out[i].line[11]);
        printf("    .byte 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", out[i].line[12], out[i].line[13], out[i].line[14], out[i].line[15]);
    }
    
    free(out);
    
    return 0;
}

void convert(FILE *fp, struct ascii *out){
    for(int i = 0; i < H; i++){
        char buf[BUF_LEN] = {0};
        fgets(buf, BUF_LEN, fp); //エラー処理省略
        
        for(int j = 0; j < W; j++){
            char c = (buf[j] == '*' ? 1 : 0);
            out->line[i] += c << (W - j - 1);
        }
    }
}
