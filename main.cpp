#include <cstring>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

const std::string RAMfilename = "RAM.txt";
typedef uint8_t bitType;

class microcycle
{
private:
    std::vector<bitType> data;
    const uint8_t capacity=8;
public:
    microcycle()
    {
        data=std::vector<bitType>(capacity,0);
    }
    friend std::ostream& operator<<(std::ostream& os, const microcycle& obj)
    {
        for(uint8_t k=0;k<obj.data.size();++k)
            os<<(int)obj.data[k];
        return os;
    }
    void strRead(const std::string &_text)
    {
        if(_text.size()!=capacity) return;
        for(uint8_t k=0;k<capacity;k++)
            data[k]=_text[k];
    }
    bitType& operator[](uint8_t _idx) {return data[_idx];}
    uint8_t getTxId(void) const {return data[0]*4+data[1]*2+data[2];}
    uint8_t getRxId(void) const {return data[3]*4+data[4]*2+data[5];}
};

class cycle
{
private:
    std::vector<microcycle> data;
    const uint8_t capacity=4;
public:
    cycle()
    {
        data.resize(capacity);
    }
    friend std::ostream& operator<<(std::ostream& os, const cycle& obj)
    {
        for(uint8_t k=0;k<obj.data.size();++k)
            os<<obj.data[k]<<"\n";
        return os;
    }
    microcycle& operator[](uint8_t _idx) {return data[_idx];}
};

class ram
{
private:
    std::vector<cycle> data;
    const uint8_t capacity=8;
public:
    ram()
    {
        data.resize(capacity);
    }
    friend std::ostream& operator<<(std::ostream& os, const ram& obj)
    {
        for(uint8_t k=0;k<obj.data.size();++k)
            os<<obj.data[k]<<"\n";
        return os;
    }
    cycle& operator[](uint8_t _idx) {return data[_idx];}
    void readFromFile(const std::string &_filename)
    {
        std::cout<<"Filename="<<_filename<<"\n";
        std::ifstream file(_filename);
        std::string str;
        uint8_t cycle=0;
        uint8_t microcycle=0;
        while(std::getline(file, str))
        {
            if(str[0]=='#') continue;
            for(uint8_t k=0;k<8;k++)
                data[cycle][microcycle][k]=str[k]-48;
            ++microcycle;
            cycle+=microcycle/4;
            microcycle%=4;
            if(cycle==0 && microcycle==0) return;
        }
    }
};

class reg
{
private:
    std::vector<bitType> data;
    uint8_t capacity;
public:
    reg(uint8_t _c)
    {
        capacity=_c;
        data=std::vector<bitType>(capacity,0);
    }
    void serialInput(const bitType &_input)
    {
        for(uint8_t k=0;k<capacity-1;++k)
            data[k]=data[k+1];
        data[capacity-1]=_input;
    }
    void parallelInput(const reg* _src)
    {
        if(_src==this) return;
        uint8_t minSize=std::min(this->capacity,_src->capacity);
        data=std::vector<bitType>(capacity,0);
        for(uint8_t k=0;k<minSize;++k)
            data[capacity-1-k]=_src->data[_src->capacity-1-k];
    }
    friend std::ostream& operator<<(std::ostream& os, const reg& obj)
    {
        for(uint8_t k=0;k<obj.capacity;++k)
            os<<(int)obj.data[k];
        return os;
    }
    bitType& operator[](uint8_t _idx) {return data[_idx];}
    bitType operator[](uint8_t _idx) const {return data[_idx];}
    void readInput(void)
    {
        std::string input;
        while(validateInput(input))
        {
            std::cout<<"Write 4 input bits:";
            std::cin>>input;
        }
        for(uint8_t k=0;k<capacity;++k)
            data[k]=input[k]-'0';
    }
    bool validateInput(const std::string &_input) // true - invalid, false - valid
    {
        if(_input.length()!=4) return true;
        for(uint8_t k=0;k<4;++k)
            if(_input[k]!='0' && _input[k]!='1') return true;
        return false;
    }
    reg& operator++()
    {
        ++data[capacity-1];
        for(uint8_t k=capacity-1;k>0;--k)
        {
            data[k-1]+=data[k]/2;
            data[k]%=2;
        }
        data[0]%=2;
        return *this;
    }
    uint8_t getValue(void) const
    {
        uint8_t rv=0;
        uint8_t power2=1;
        for(int8_t k=capacity-1;k>=0;--k)
        {
            rv+=data[k]*power2;
            power2*=2;
        }
        return rv;
    }
    void fromValue(uint8_t _value)
    {
        uint8_t pow2capacity;
        pow2capacity=1<<capacity;
        _value%=pow2capacity;
        for(int8_t k=capacity-1;k>=0;--k)
        {
            data[k]=_value%2;
            _value/=2;
        }
    }
};

class alu
{
private:
    reg outReg;
public:
    alu():outReg(4)
    {
    }
    reg &calculate(reg &_R1, reg &_R2, const reg &_command)
    {
        uint8_t r1v=_R1.getValue();
        uint8_t r2v=_R2.getValue();
        uint8_t M=_command[0];
        uint8_t S=_command.getValue()-M*16;

        if(M==0)
        {
            switch (S)
            {
            case 0:
                outReg.fromValue(0);
                break;
            case 1:
                outReg.fromValue(r1v/r2v);
                break;
            case 2:
                outReg.fromValue(r1v*r2v-1);
                break;
            case 3:
                outReg.fromValue(r1v+r2v);
                break;
            case 4:
                outReg.fromValue(r1v*r1v-r2v*r2v);
                break;
            case 5:
                outReg.fromValue(r1v*2);
                break;
            case 6:
                outReg.fromValue((r1v+r2v)/2);
                break;
            case 7:
                outReg.fromValue(r1v*r2v);
                break;
            case 8:
                outReg.fromValue(r1v%r2v);
                break;
            case 9:
                outReg.fromValue(r1v/2);
                break;
            case 10:
                outReg.fromValue((r1v*r2v)-(r1v+r2v));
                break;
            case 11:
                outReg.fromValue(r1v-r2v);
                break;
            case 12:
                outReg.fromValue((r1v-r2v)*(r1v-r2v));
                break;
            case 13:
                outReg.fromValue(r1v);
                break;
            case 14:
                outReg.fromValue(r1v*r1v);
                break;
            case 15:
                outReg.fromValue(r1v-1);
                break;
            default:
                break;
            }
        }
        else
        {
            for(uint8_t k=0;k<4;++k)
            {
                switch (S)
                {
                case 0:
                    outReg[k]=!_R1[k];
                    break;
                case 1:
                    outReg[k]=!(_R1[k] & _R2[k]);
                    break;
                case 2:
                    outReg[k]=!_R1[k] | _R2[k];
                    break;
                case 3:
                    outReg[k]=1;
                    break;
                case 4:
                    outReg[k]=(_R1[k] + _R2[k])%2;
                    break;
                case 5:
                    outReg[k]=!(_R1[k] == _R2[k]);
                    break;
                case 6:
                    outReg[k]=(!_R1[k]) | _R2[k];
                    break;
                case 7:
                    outReg[k]=_R1[k] | _R2[k];
                    break;
                case 8:
                    outReg[k]=!((!_R1[k]) | _R2[k]);
                    break;
                case 9:
                    outReg[k]=!((!_R1[k]) & _R2[k]);
                    break;
                case 10:
                    outReg[k]=_R1[k] & _R2[k];
                    break;
                case 11:
                    outReg[k]=!((_R1[k] + _R2[k])%2);
                    break;
                case 12:
                    outReg[k]=!_R2[k];
                    break;
                case 13:
                    outReg[k]=!(_R1[k] | _R2[k]);
                    break;
                case 14:
                    outReg[k]=_R1[k] == _R2[k];
                    break;
                case 15:
                    outReg[k]=!((_R1[k] + _R2[k])%2);
                    break;
                default:
                    break;
                }
            }
        }
        return outReg;
    }
};

class device
{
private:
    reg A, B, C, RI, Rc, input, R1, R2, command;
    reg* txPtr, *rxPtr;
    ram memory;
    alu aluUnit;
    bool end;
    bool nowait;
public:
    device(const std::string &filename=RAMfilename):A(4), B(4), C(4), R1(4), R2(4), command(5), Rc(2), RI(3), input(4)
    {
        memory.readFromFile(filename);
        end=false;
        nowait=false;
    }
    void displayState() const
    {
        std::cout<<"RI: "<<RI<<" Rc:"<<Rc<<" A: "<<A<<" B: "<<B<<" C:"<<C<<" MS3S2S1S0="<<command<<"\n";
    }
    reg* getTxPtr(uint8_t _id)
    {
        if(_id==0) {return &RI;}
        if(_id==1) {return &A;}
        if(_id==2) {return &B;}
        if(_id==3) {return &C;}
        if(_id==4) {return &aluUnit.calculate(R1,R2,command);}
        if(_id==5) {input.readInput(); return &input;}
        return nullptr;
    }
    reg* getRxPtr(uint8_t _id)
    {
        if(_id==0) {return &RI;}
        if(_id==1) {return &A;}
        if(_id==2) {return &B;}
        if(_id==3) {return &C;}
        if(_id==4) {return &R1;}
        if(_id==5) {return &R2;}
        return nullptr;
    }
    void doStep()
    {
        microcycle uc=memory[RI.getValue()][Rc.getValue()];
        uint8_t txId=uc.getTxId();
        uint8_t rxId=uc.getRxId();
        txPtr=getTxPtr(txId);
        rxPtr=getRxPtr(rxId);
        if(rxId==6)
            std::cout<<"Output:"<<*txPtr<<"\n";
        else
            rxPtr->parallelInput(txPtr);

        // M S3 S2 S1 S0
        if(Rc.getValue()==0 || Rc.getValue()==1 || Rc.getValue()==2) command.serialInput(uc[6]);
        if(Rc.getValue()==0 || Rc.getValue()==1) command.serialInput(uc[7]);

        ++Rc;
        if(Rc.getValue()==0) ++RI;
        if(RI.getValue()==0 && Rc.getValue()==0) end=true;
    }
    void startWork()
    {
        char kbInp;
        do{
            displayState();
            doStep();
            if(!nowait)
            {
                kbInp=std::getchar();
                if(kbInp=='q') break;
                if(kbInp=='n') nowait=true;
            }
            //if(kbInp=='a' || kbInp=='A') std::cout<<"RejA: "<<A<<"\n";
            //if(kbInp=='b' || kbInp=='B') std::cout<<"RejB: "<<B<<"\n";
            //if(kbInp=='c' || kbInp=='C') std::cout<<"RejC: "<<C<<"\n";
        }while(!end);
    }
};

int main(int argc, char* argv[])
{
    if(argc==2)
    {
        const std::string arg = argv[1];
        device dataBus(arg);
        dataBus.startWork();
    }
    else
    {
        device dataBus;
        dataBus.startWork();
    }
    return 0;
}
