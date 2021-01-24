#ifndef RANDOM_ACCESS_RabinHash_H
#define RANDOM_ACCESS_RabinHash_H

/*Derived from "Some Applications of Rabin's Fingerprinting Method"
by Andrei Broder, adapted from the Sean Owen's Java version, which
is in url:"https://sourceforge.net/projects/rabinhash/"
*/

#include <string>
#include <cstring>

using std::string;

class RabinHash
{
    public:
        RabinHash(const int poly = DEFAULT_IRREDUCIBLE_POLY);
        virtual ~RabinHash();

        int getP() { return P;}
        friend bool operator== (RabinHash, RabinHash);

        unsigned int operator() (const unsigned char *A, const int offset, const int len, int w);
        unsigned int operator() (const char* A, int len){ return rabinHashFunc32(A,len);}
        unsigned int operator() (const char* A){return rabinHashFunc32(A, strlen(A));}
        unsigned int operator() (const string s){ return rabinHashFunc32(s);}

        //unsigned int rabinfun(const char *A){return rabinHashFunc32(A, strlen(A));}

    private:
        void initTables();
        int computeWShifted(const int w);

        int rabinHashFunc32(const unsigned char *A, const int offset, const int len, int w);
        int rabinHashFunc32(const char* A, int len);
        int rabinHashFunc32(const string s){ return rabinHashFunc32(s.c_str(), s.length());}


    private:
        //DEFAULT_IRREEDUCIBLE_POLY ����t^32 + t^7 + t^3 + t^2 + 1
        static const int DEFAULT_IRREDUCIBLE_POLY = 0x8D;
        static const int P_DEGREE = 32;
        static const int X_P_DEGREE = 1 << (P_DEGREE - 1);
        static const int READ_BUFFER_SIZE = 1024;

        //P����GF(2)�ϵ�32�׶���ʽ����int����ʾ
        const int P;
        int *table32, *table40, *table48, *table56;
};

unsigned int RabinHashFunc(const char *str); //�ӿ�
#endif // RABINHASH_H
