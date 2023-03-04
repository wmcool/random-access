#include "RabinHash.h"

RabinHash::RabinHash(const int poly ) : P(poly)
{
    initTables();
}

RabinHash::~RabinHash()
{
    delete[] table32;
    delete[] table40;
    delete[] table48;
    delete[] table56;
}

void RabinHash::initTables()
{
    int *mods = new int[P_DEGREE];

    //Ŀ�ģ���ǰ��� x^(k+i) mod P(t)��ʹ��mods[i] == x^(P_DEGREE + i)
    mods[0] = P;
    for(int i = 1; i < P_DEGREE; ++i){
        const int lastMod = mods[i-1];
        int thisMod = lastMod << 1; // x^i == x(x^(i-1)) mod P;

        //���x^(i-1)�����λΪ1�����������ƺ����λ��1�ᱻ����
        //����Ϊx^k mod P == P, ����Ӧ�ü���P
        if ( (lastMod & X_P_DEGREE) != 0 )
            thisMod ^= P;
        mods[i] = thisMod;
    }

    // table32[i] = Q32, ���� Q32 = (b0*x^39 + b1*x^38 + ... + b7*x^32) mod P;
    table32 = new int[256];
    // table40[i] = Q40, ���� Q40 = (b0*x^47 + b1*x^46 + ... + b7*x^40) mod P;
    table40 = new int[256];
    table48 = new int[256];
    table56 = new int[256];

    memset(table32, 0, sizeof(int)*256);
    memset(table40, 0, sizeof(int)*256);
    memset(table48, 0, sizeof(int)*256);
    memset(table56, 0, sizeof(int)*256);

    for (int i = 0; i < 256; ++i){
        int c = i;
        for (int j = 0; j < 8 && c > 0; ++j){
            if ( (c & 1) != 0 ){
                table32[i] ^= mods[j];
                table40[i] ^= mods[j+8];
                table48[i] ^= mods[j+16];
                table56[i] ^= mods[j+24];
            }

            c >>= 1;
        }
    }

    delete[] mods;
}

int RabinHash::computeWShifted(const int w)
{
    return table32[w         & 0xFF] ^
           table40[(w >> 8)  & 0xFF] ^
           table48[(w >> 16) & 0xFF] ^
           table56[(w >> 24) & 0xFF] ;
}

int RabinHash::rabinHashFunc32(const unsigned char *A, const int offset, const int len, int w)
{
    int s = offset;

    //���ȣ��������ֽڣ�s.t. ʣ�µ��ֽ����ܹ�����4
    const int starterBytes = len % 4;
    if (starterBytes){
        const int max = offset + starterBytes;
        while( s < max){
            w = (w << 8) ^ (A[s] & 0xFF);
            ++s;
        }
    }

    const int max = offset + len;
    while( s < max){
        w = computeWShifted(w)       ^
            ( A[s] << 24 )           ^
            ( (A[s+1] & 0xFF) << 16) ^
            ( (A[s+2] & 0xFF) << 8 ) ^
            ( (A[s+3] & 0xFF) );
        s += 4;

    }

    return w;
}


int RabinHash::rabinHashFunc32(const char* A, int len)
{
    int w, s;

    //�ַ��ĸ�����Ϊ���������ȴ������ַ�
    if(len % 2){
        w = A[0] & 0xFFFF;
        s = 1;
    }else{
        w = 0;
        s = 0;
    }

    while (s < len){
        w = computeWShifted(w)        ^
            ( (A[s] & 0xFFFF) << 16 ) ^
            ( A[s+1] & 0xFFFF );
        s += 2;
    }

    return w;
}

unsigned int RabinHash::operator() (const unsigned char *A, const int offset, const int len, int w)
{
    return rabinHashFunc32(A, offset, len, w);
}

bool operator== (RabinHash rx, RabinHash ry)
{
    return (rx.P == ry.P);
}

unsigned int RabinHashFunc(const char *str)
{
    RabinHash rabin;
    return rabin(str);
}

//#define RABINHASH_TEST
#ifdef RABINHASH_TEST

#include <iostream>
using namespace std;

int main()
{
    RabinHash rh;
    RabinHash rh2(0xea);
    RabinHash rh3;
    char *a = "34erfd012";
    char *b = "1234567sd";

    cout << a << "'s rabin fingerprint (default poly): "  << rh(a)  << endl;
    cout << a << "'s rabin fingerprint (poly == 0xea): " << rh2(a,9) << endl;
    cout << a << "'s rabin fingerprint (default poly): " << rh3(a,9)  << endl;
    cout << b << "'s rabin fingerprint (default poly): " << rh(b,9)  << endl;
    cout << a << "'s rabin fingerprint (default poly): " << rh(a,9)  << endl;
 /*   cout << "Rabin Hash Functions rh2 and rh "
         << ( (rh2 == rh) ? "use the same polynomial."
         : "do not have the same polynomial.") << endl;
*/
    return 0;
}

#endif // RABINHASH_TEST
