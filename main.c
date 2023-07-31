#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int Len = 16;
int Qubit = 6;

void SumGate(bool c, bool x, bool y, bool *Out){
    int total = (int)c + (int)x + (int)y;
    //基本的な3in2outの加算器と同じ処理
    if(total == 0){
        Out[0] = 0;
        Out[1] = 0;
    }
    else if(total == 1){
        Out[0] = 1;
        Out[1] = 0;
    }
    else if(total == 2){
        Out[0] = 0;
        Out[1] = 1;
    }
    else if(total == 3){
        Out[0] = 1;
        Out[1] = 1;
    }
    else{
        printf("error\n");
    }
}

//加算処理
void Sum(bool *x, bool *y, bool *z){
    bool out[2];
    bool c;
    int i;
    for(i = Len-1; i >= 0; i--){
        if(i == Len-1 || i == Len/2-1) c = 0;
        SumGate(c,x[i],y[i],out);
        z[i] = out[0];
        c = out[1];
    }
}

//符号を反転
void SignChange(bool *x, bool *z){
    int i;
    bool one[Len];

    for(i = 0; i < Len; i++) x[i] ^= 1;
    for(i = 0; i < Len; i++){
        one[i] = 0;
        if(i == (Len/2)-1 || i == Len-1) one[i] = 1;
    }
    Sum(x,one,z);
}

void MulGate(bool x, bool y0, bool y1, bool *Out){
    //PowerPointerで紹介した乗算器の処理を行う
    if(x == 0){
        Out[0] = 0;
        Out[1] = 0;
    }
    else if(y1 == 1){
        Out[0] = 0;
        Out[1] = 1;
    }
    else if(y0 == 0 && y1 == 0){
        Out[0] = 0;
        Out[1] = 0;
    }
    else if(y0 == 1 && y1 == 0){
        Out[0] = 1;
        Out[1] = 0;
    }
}

//乗算処理
void Mul(bool *x, bool *y, bool *z){
    int i;
    //乗算器による計算
    bool MulOut[Len][2];
    for(i = 0; i < Len; i++){
        MulGate(x[i],y[0],y[1],MulOut[i]);
    }

    //ORゲートに入れる前のビットの交換（整理）
    bool ORIn[Len][2];
    for(i = 0; i < Len; i++){
        ORIn[i][0] = MulOut[i][0];
        if(i < Len/2) ORIn[i+(Len/2)][1] = MulOut[i][1];
        else if(i != Len-1) ORIn[i-((Len/2)-1)][1] = MulOut[i][1];
        else ORIn[0][1] = MulOut[Len/2][1];
    }

    //ORゲートの処理
    bool OROut[Len];
    for(i = 0; i < Len; i++){
        OROut[i] = ORIn[i][0] + ORIn[i][1];
        if(OROut[i] == 2){
            printf("error!");
        }
    }

    //yが負の数だった時の-1の乗算
    bool ys = (int)y[0] * (int)y[1];
    if(ys) SignChange(OROut,z);
    else{
        for(i = 0; i < Len; i++) z[i] = OROut[i];
    }

    /*//ビット反転(正の場合維持)
    for(i = 0; i < Len; i++) OROut[i] ^= ys;
    //+1(正の場合+0)処理
    bool one[Len];
    for(i = 0; i < Len; i++){
        one[i] = 0;
        if(i == (Len/2)-1 || i == Len-1) one[i] = ys;
    }
    Sum(OROut,one,z);*/
}



//複素数の和
void ComplexSum(bool x[][Len], bool y[][Len], bool z[][Len]){
    Sum(x[0],y[0],z[0]);
    Sum(x[1],y[1],z[1]);
}

//複素数の積
void ComplexMul(bool x[][Len], bool y[][2], bool z[][Len]){
    bool RR[Len], RI[Len], IR[Len], II[Len], MinusII[Len];
    int i;

    //実部
    Mul(x[0],y[0],RR);
    Mul(x[1],y[1],II);
    SignChange(II,MinusII);
    Sum(RR,MinusII,z[0]);
    
    //虚部
    Mul(x[0],y[1],RI);
    Mul(x[1],y[0],IR);
    Sum(RI,IR,z[1]);
}

void t(bool StateR[][Len],bool StateI[][Len],int bit){
    unsigned int i,j;
    unsigned int counter1 = 1, counter2 = 1;    // counter1は注目ビットより前,counter2は注目ビットより後ろ
    bool Zero[2][Len], One[2][Len], AnswerZero[2][Len],AnswerOne[2][Len];
    bool y01[2][2] = {{1,0},{0,0}}, y10i10[2][2] = {{0,1},{0,1}};
    for(i = 0; i < bit - 1; i++) counter1 *= 2;
    for(i = 0; i < Qubit - bit; i++) counter2 *= 2;
    for(i = 0; i < counter2; i++){
        for(j = 0; j < counter1; j++){
            for(unsigned int k = 0; k < Len; k++){
                Zero[0][k] = StateR[j + counter1 * 2 * i][k];
                Zero[1][k] = StateI[j + counter1 * 2 * i][k];
                One[0][k] = StateR[j + counter1 * 2 * i + counter1][k];
                One[1][k] = StateI[j + counter1 * 2 * i + counter1][k];
            }
            ComplexMul(Zero, y01, AnswerZero);
            ComplexMul(One, y10i10, AnswerOne);
            for(unsigned int k = 0; k < Len; k++){
                StateR[j + counter1 * 2 * i][k] = AnswerZero[0][k];
                StateI[j + counter1 * 2 * i][k] = AnswerZero[1][k];
                StateR[j + counter1 * 2 * i + counter1][k] = AnswerOne[0][k];
                StateI[j + counter1 * 2 * i + counter1][k] = AnswerOne[1][k];
            }
        }
    }
}

void h(bool StateR[][Len],bool StateI[][Len],int bit){
    unsigned int i,j;
    unsigned int counter1 = 1, counter2 = 1;    // counter1は注目ビットより前,counter2は注目ビットより後ろ
    bool Zero[2][Len], One[2][Len],AnswerZero[2][Len],AnswerOne[2][Len],
     AnswerZero1[2][Len],AnswerZero2[2][Len],AnswerOne1[2][Len],AnswerOne2[2][Len];
    bool y10[2][2] = {{0,1},{0,0}}, y11[2][2] = {{1,1},{0,0}};
    for(i = 0; i < bit - 1; i++) counter1 *= 2;
    for(i = 0; i < Qubit - bit; i++) counter2 *= 2;
    for(i = 0; i < counter2; i++){
        for(j = 0; j < counter1; j++){
            for(unsigned int k = 0; k < Len; k++){
                Zero[0][k] = StateR[j + counter1 * 2 * i][k];
                Zero[1][k] = StateI[j + counter1 * 2 * i][k];
                One[0][k] = StateR[j + counter1 * 2 * i + counter1][k];
                One[1][k] = StateI[j + counter1 * 2 * i + counter1][k];
            }
            ComplexMul(Zero, y10, AnswerZero1);
            ComplexMul(One, y10, AnswerZero2);
            ComplexSum(AnswerZero1,AnswerZero2,AnswerZero);
            ComplexMul(Zero, y10, AnswerOne1);
            ComplexMul(One, y11, AnswerOne2);
            ComplexSum(AnswerOne1,AnswerOne2,AnswerOne);
            for(unsigned int k = 0; k < Len; k++){
                StateR[j + counter1 * 2 * i][k] = AnswerZero[0][k];
                StateI[j + counter1 * 2 * i][k] = AnswerZero[1][k];
                StateR[j + counter1 * 2 * i + counter1][k] = AnswerOne[0][k];
                StateI[j + counter1 * 2 * i + counter1][k] = AnswerOne[1][k];
            }
        }
    }
}

void cx(bool StateR[][Len],bool StateI[][Len],int control,int target){
    unsigned int i,j,k,l;
    unsigned int counter1 = 1, counter2 = 1, counter3 = 1;
    bool ZZ[2][Len], ZO[2][Len],OZ[2][Len], OO[2][Len],AnswerZZ[2][Len],AnswerZO[2][Len],
     AnswerOZ[2][Len],AnswerOO[2][Len];
    bool y01[2][2] = {{1,0},{0,0}};
    if(control < target){
        for(i = 0; i < control - 1; i++) counter1 *= 2;
        for(i = 0; i < target - control - 1; i++) counter2 *= 2;
        for(i = 0; i < Qubit - target; i++) counter3 *= 2;

        for(i = 0; i < counter3; i++){
            for(j = 0; j < counter2; j++){
                for(k = 0; k < counter1; k++){
                    for(l = 0; l < Len; l++){
                        ZZ[0][l] = StateR[k + counter1 * 2 * j
                            + counter1 * counter2 * 4 * i][l];
                        ZZ[1][l] = StateI[k + counter1 * 2 * j
                            + counter1 * counter2 * 4 * i][l];
                        ZO[0][l] = StateR[k + counter1 * 2 * j + counter1
                            + counter1 * counter2 * 4 * i][l];
                        ZO[1][l] = StateI[k + counter1 * 2 * j + counter1
                            + counter1 * counter2 * 4 * i][l];
                        OZ[0][l] = StateR[k + counter1 * 2 * j 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l];
                        OZ[1][l] = StateI[k + counter1 * 2 * j 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l];
                        OO[0][l] = StateR[k + counter1 * 2 * j + counter1 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l];
                        OO[1][l] = StateI[k + counter1 * 2 * j + counter1 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l];
                    }

                    ComplexMul(ZZ, y01, AnswerZZ);
                    ComplexMul(ZO, y01, AnswerOO);
                    ComplexMul(OZ, y01, AnswerOZ);
                    ComplexMul(OO, y01, AnswerZO);

                    for(l = 0; l < Len; l++){
                        StateR[k + counter1 * 2 * j
                            + counter1 * counter2 * 4 * i][l] = AnswerZZ[0][l];
                        StateI[k + counter1 * 2 * j
                            + counter1 * counter2 * 4 * i][l] = AnswerZZ[1][l];
                        StateR[k + counter1 * 2 * j + counter1
                            + counter1 * counter2 * 4 * i][l] = AnswerZO[0][l];
                        StateI[k + counter1 * 2 * j + counter1
                            + counter1 * counter2 * 4 * i][l] = AnswerZO[1][l];
                        StateR[k + counter1 * 2 * j 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l] = AnswerOZ[0][l];
                        StateI[k + counter1 * 2 * j 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l] = AnswerOZ[1][l];
                        StateR[k + counter1 * 2 * j + counter1 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l] = AnswerOO[0][l];
                        StateI[k + counter1 * 2 * j + counter1 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l] = AnswerOO[1][l];
                    }
                }
            }
        }
    }
    else if(control > target){
        for(i = 0; i < target - 1; i++) counter1 *= 2;
        for(i = 0; i < control - target - 1; i++) counter2 *= 2;
        for(i = 0; i < Qubit - control; i++) counter3 *= 2;

        for(i = 0; i < counter3; i++){
            for(j = 0; j < counter2; j++){
                for(k = 0; k < counter1; k++){
                    for(l = 0; l < Len; l++){
                        ZZ[0][l] = StateR[k + counter1 * 2 * j
                            + counter1 * counter2 * 4 * i][l];
                        ZZ[1][l] = StateI[k + counter1 * 2 * j
                            + counter1 * counter2 * 4 * i][l];
                        ZO[0][l] = StateR[k + counter1 * 2 * j + counter1
                            + counter1 * counter2 * 4 * i][l];
                        ZO[1][l] = StateI[k + counter1 * 2 * j + counter1
                            + counter1 * counter2 * 4 * i][l];
                        OZ[0][l] = StateR[k + counter1 * 2 * j 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l];
                        OZ[1][l] = StateI[k + counter1 * 2 * j 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l];
                        OO[0][l] = StateR[k + counter1 * 2 * j + counter1 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l];
                        OO[1][l] = StateI[k + counter1 * 2 * j + counter1 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l];
                    }

                    ComplexMul(ZZ, y01, AnswerZZ);
                    ComplexMul(ZO, y01, AnswerZO);
                    ComplexMul(OZ, y01, AnswerOO);
                    ComplexMul(OO, y01, AnswerOZ);

                    for(l = 0; l < Len; l++){
                        StateR[k + counter1 * 2 * j
                            + counter1 * counter2 * 4 * i][l] = AnswerZZ[0][l];
                        StateI[k + counter1 * 2 * j
                            + counter1 * counter2 * 4 * i][l] = AnswerZZ[1][l];
                        StateR[k + counter1 * 2 * j + counter1
                            + counter1 * counter2 * 4 * i][l] = AnswerZO[0][l];
                        StateI[k + counter1 * 2 * j + counter1
                            + counter1 * counter2 * 4 * i][l] = AnswerZO[1][l];
                        StateR[k + counter1 * 2 * j 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l] = AnswerOZ[0][l];
                        StateI[k + counter1 * 2 * j 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l] = AnswerOZ[1][l];
                        StateR[k + counter1 * 2 * j + counter1 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l] = AnswerOO[0][l];
                        StateI[k + counter1 * 2 * j + counter1 
                            + counter1 * counter2 * 2 + counter1 * counter2 * 4 * i][l] = AnswerOO[1][l];
                    }
                }
            }
        }
    }
}

int main(void){
    unsigned int StateNum = 1;
    unsigned int i,j;
    for(i = 0; i < Qubit; i++) StateNum *= 2;
    bool StateR[StateNum][Len],StateI[StateNum][Len];
    for(i = 0; i < StateNum; i++){
        for(j = 0; j < Len; j++){
            StateR[i][j] = 0;
            StateI[i][j] = 0;
        }
    }
    StateR[0][1] = 1;

    //回路
    
    h(StateR,StateI,1);
    h(StateR,StateI,2);
    h(StateR,StateI,3);
    cx(StateR,StateI,1,4);
    cx(StateR,StateI,2,5);
    cx(StateR,StateI,3,6);
    cx(StateR,StateI,2,5);
    cx(StateR,StateI,2,6);
    h(StateR,StateI,1);
    h(StateR,StateI,2);
    h(StateR,StateI,3);
    for(i = 0; i < StateNum; i++){
        if(i < 10) printf("%d  ",i);
        else printf("%d ",i);
        for(j = 0; j < Len; j++){
            printf("%d",(int)StateR[i][j]);
            if(j == (Len/2)-1) printf(" ");
        }
        printf(" + ");
        for(j = 0; j < Len; j++){
            printf("%d",(int)StateI[i][j]);
            if(j == (Len/2)-1) printf(" ");
        }
        printf("i\n");
    }
    
    return 0;
}
