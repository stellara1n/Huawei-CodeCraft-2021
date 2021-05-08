#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/time.h>
using namespace std;
//#define OFFLINE

const string inputPath = "./data/training-1.txt";
#ifdef OFFLINE
#define _FILE "main.cpp"
#define PRINTFUNCTION(format, ...)    fprintf(stdout, format, ##__VA_ARGS__);
#define LOG_FMT             "%-24s | "
#define LOG_INFO(message, args...)  PRINTFUNCTION(LOG_FMT message "\n" , timenow(), ## args)
#else
#define LOG_INFO(message, args...)
#endif
static inline char *timenow() {
    static char buffer[64];
    time_t now = time(0);
    tm *ltm = localtime(&now);
    sprintf(buffer,"[%d-%d-%d %d:%d:%d]", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return buffer;
}

typedef long long ll;
int N,M,T,K;
int nowTime;
ll purchaseCostSum = 0, dailyCostSum = 0;
ll salesSum = 0;
int cpuSum = 1, memSum = 1;
int cpuNow = 1, memNow = 1;
int vmNow = 0;
int cpuBuy = 1, memBuy = 1;

const int uselessspace = 20;
int totalmin = 2000000;
int totalmin_cpu;
int totalmin_mem;



int magic(int cpu, int mem){
    return cpu + mem;
}
int magic(int cpu1, int mem1, int cpu2, int mem2){
    return cpu1 + mem1 + cpu2 + mem2;
}

struct VMType{
    string name;
    int cpuNum,memNum;
    bool isDual;
    VMType(){}
    VMType(string _name, int _cpuNum, int _memNum, bool _isDual){
        name = _name;
        cpuNum = _cpuNum;
        memNum = _memNum;
        isDual = _isDual;
    }
};
struct VM{
    int id;
    int index;
    VMType type;
    int addTime,lifeCycle;
    int baseOffer;
    int myOffer, othersOffer;
    int cost;
    bool obtain;
    bool bind;
    bool oppbind;
    int bindindex;
    pair<int,int> placement;
    VM(){}
    VM(int _id, int _index, const VMType& _type, int _addTime, int _lifeCycle, int _baseOffer){
        id = _id;
        index = _index;
        type = _type;
        addTime = _addTime;
        lifeCycle = _lifeCycle;
        baseOffer = _baseOffer;
        myOffer = othersOffer = -1;
        cost = -1;
        obtain = true;
        placement = make_pair(-1,-1);
        bind = false;oppbind = false;
        bindindex = -1;
    }
    int magic(){
        return type.cpuNum + type.memNum;
    }
};

struct PMType{
    string name;
    int cpuNum,memNum;
    int purchaseCost,dailyCost;
    PMType(){}
    PMType(string _name, int _cpuNum, int _memNum, int _purchaseCost, int _dailyCost){
        name = _name;
        cpuNum = _cpuNum;
        memNum = _memNum;
        purchaseCost = _purchaseCost;
        dailyCost = _dailyCost;
    }
};
struct PM{
    int id;
    PMType type;
    pair<int,int> A,B;
    unordered_map<int,int> vmSet;
    list<int>::iterator p0,p1,p2;
    int purchaseTime;
    PM(){}
    PM(int _id, const PMType& _type, int _purchaseTime){
        id = _id;
        type = _type;
        purchaseTime = _purchaseTime;
        A = B = {_type.cpuNum/2, _type.memNum/2};
    }
    int remain(){
        return A.first+A.second+B.first+B.second;
    }
    int magicA(){
        return magic(A.first, A.second);
    }
    int magicB(){
        return magic(B.first, B.second);
    }
    int magicAB(){
        return magic(A.first, A.second, B.first, B.second);
    }
    bool isEmptyA(){
        return A.first==type.cpuNum/2&&A.second==type.memNum/2;
    }
    bool isEmptyB(){
        return B.first==type.cpuNum/2&&B.second==type.memNum/2;
    }
    bool isEmpty(){
        return isEmptyA()&&isEmptyB();
    }
    double ratio(){
        return 1.0-(A.first+A.second+B.first+B.second)*1.0/(type.cpuNum+type.memNum);
    }
    string info(){
        return to_string(id) + " " + to_string(ratio()) +"A: "+to_string(type.cpuNum/2-A.first)+"/"+to_string(type.cpuNum/2)+"  "+to_string(type.memNum/2-A.second)+"/"+to_string(type.memNum/2)
        +"  B: "+to_string(type.cpuNum/2-B.first)+"/"+to_string(type.cpuNum/2)+"  "+to_string(type.memNum/2-B.second)+"/"+to_string(type.memNum/2);
    }
};
struct Request{
    int type;
    int id;
    Request(){}
    Request(int _type, int _id){
        type = _type;
        id = _id;
    }
};

PMType pmtypes[105]; int pmtypeCnt = 0;
VMType vmtypes[1005]; int vmtypeCnt = 0;
unordered_map<string, int> vmtypeMap;
VM vms[100005];  int vmCnt = 0;
unordered_map<int, int> vmMap;
int requestOffset[1005];
Request requests[200005];  int requestCnt = 0;

PM pms[100005];
int pmIDs[100005];
unordered_map<int, int> pmIDmap;
int pmCnt = 0;

list<int> *singleList, *dualList;
int singleListSize,dualListSize;



void read(){
#ifdef OFFLINE
    freopen(inputPath.c_str(), "rb", stdin);
#endif // OFFLINE
    char buffer[60];
    scanf("%d\n",&N);
    LOG_INFO("N=%d",N);
    for(int i=0;i<N;i++){
        string name = "";
        int cpu=0, mem=0, purchasecost=0, dailycost=0;
        gets(buffer);
        char* pos = buffer;
        pos++;
        while(*pos!=','){
            name += *pos;
            pos++;
        }
        pos += 2;
        while(*pos!=','){
            cpu = cpu*10 + *pos-'0';
            pos++;
        }
        pos += 2;
        while(*pos!=','){
            mem = mem*10 + *pos-'0';
            pos++;
        }
        pos += 2;
        while(*pos!=','){
            purchasecost = purchasecost*10 + *pos-'0';
            pos++;
        }
        pos += 2;
        while(*pos!=')'){
            dailycost = dailycost*10 + *pos-'0';
            pos++;
        }
        pos += 2;
        pmtypes[pmtypeCnt++] = PMType(name,cpu,mem,purchasecost,dailycost);
    }

    scanf("%d\n",&M);
    LOG_INFO("M=%d",M);
    for(int i=0;i<M;i++){
        string name="";
        int cpu=0,mem=0;
        bool dual=false;
        gets(buffer);
        char* pos = buffer;
        pos++;
        while(*pos!=','){
            name += *pos;
            pos++;
        }
        pos += 2;
        while(*pos!=','){
            cpu = cpu*10 + *pos-'0';
            pos++;
        }
        pos += 2;
        while(*pos!=','){
            mem = mem*10 + *pos-'0';
            pos++;
        }
        pos += 2;
        dual = (*pos=='1')?true:false;
        pos += 3;
        vmtypes[vmtypeCnt] = VMType(name, cpu, mem,dual);
        vmtypeMap[name] = vmtypeCnt;
        vmtypeCnt++;
    }

    scanf("%d %d\n",&T,&K);
    LOG_INFO("T=%d  K=%d",T,K);
    requestOffset[0] = 0;
    for(int t=0;t<K;t++){
        int R;
        scanf("%d\n",&R);
        requestOffset[t+1] = requestOffset[t] + R;
        for(int r=0;r<R;r++){
            gets(buffer);
            char* pos = buffer;
            int type = 0;
            int id = 0;
            pos++;
            type = int(*pos=='a');
            while(*pos!=','){
                pos++;
            }
            pos += 2;
            if(type==1){
                string name = "";
                int lifecycle = 0;
                int offer = 0;
                while(*pos!=','){
                    name += *pos;
                    pos++;
                }
                pos += 2;
                while(*pos!=','){
                    id = id*10 + *pos-'0';
                    pos++;
                }
                pos += 2;
                while(*pos!=','){
                    lifecycle = lifecycle*10 + *pos-'0';
                    pos++;
                }
                pos += 2;
                while(*pos!=')'){
                    offer = offer*10 + *pos-'0';
                    pos++;
                }
                pos += 2;
                vms[vmCnt] = VM(id, vmCnt, vmtypes[vmtypeMap[name]], t, lifecycle, offer);
                vmMap[id] = vmCnt++;
                requests[requestCnt++] = Request(type, id);
            }
            else{
                while(*pos!=')'){
                    id = id*10 + *pos-'0';
                    pos++;
                }
                pos += 2;
                requests[requestCnt++] = Request(type, id);
            }
        }
    }
    //  prepare
    int range = 0;
    for(PMType& pmtype: pmtypes){
        range = max(range, magic(pmtype.cpuNum/2, pmtype.memNum/2));
    }
    singleList = new list<int>[range+1];
    singleListSize = range+1;
    range = 0;
    for(PMType& pmtype: pmtypes){
        range = max(range, magic(pmtype.cpuNum, pmtype.memNum));
    }
    dualList = new list<int>[range+1];
    dualListSize = range+1;


}
void readOneDay(int t){
    fflush(stdout);
    char buffer[60];
    int R;
    scanf("%d\n",&R);
    requestOffset[t+1] = requestOffset[t] + R;
    for(int r=0;r<R;r++){
        gets(buffer);
        char* pos = buffer;
        int type = 0;
        int id = 0;
        pos++;
        type = int(*pos=='a');
        while(*pos!=','){
            pos++;
        }
        pos += 2;
        if(type==1){
            string name = "";
            int lifecycle = 0;
            int offer = 0;
            while(*pos!=','){
                name += *pos;
                pos++;
            }
            pos += 2;
            while(*pos!=','){
                id = id*10 + *pos-'0';
                pos++;
            }
            pos += 2;
            while(*pos!=','){
                lifecycle = lifecycle*10 + *pos-'0';
                pos++;
            }
            pos += 2;
            while(*pos!=')'){
                offer = offer*10 + *pos-'0';
                pos++;
            }
            pos += 2;
            vms[vmCnt] = VM(id, vmCnt, vmtypes[vmtypeMap[name]], t, lifecycle, offer);
            vmMap[id] = vmCnt++;
            requests[requestCnt++] = Request(type, id);
        }
        else{
            while(*pos!=')'){
                id = id*10 + *pos-'0';
                pos++;
            }
            pos += 2;
            requests[requestCnt++] = Request(type, id);
        }
    }
}

void print(string s){
#ifndef OFFLINE
    cout<<s<<endl;
#endif // OFFLINE
}
bool fit(VM& vm, PMType& pmtype, int mode){
    int cpuNeed = vm.type.cpuNum;
    int memNeed = vm.type.memNum;
    if(mode==2){
        cpuNeed /= 2;
        memNeed /= 2;
    }
    return pmtype.cpuNum/2>=cpuNeed&&pmtype.memNum/2>=memNeed;
}
bool fit(VM& vm, PM& pm, int mode){
    int cpuNeed = vm.type.cpuNum;
    int memNeed = vm.type.memNum;
    if(mode==2){
        cpuNeed /= 2;
        memNeed /= 2;
        if(pm.A.first>=cpuNeed&&pm.A.second>=memNeed&&pm.B.first>=cpuNeed&&pm.B.second>=memNeed)
            return true;
        else
            return false;
    }
    else if(mode==0){
        bool placeA = (pm.A.first>=cpuNeed&&pm.A.second>=memNeed);
        if(placeA)
            return true;
        else
            return false;
    }
    else if(mode==1){
        bool placeB = (pm.B.first>=cpuNeed&&pm.B.second>=memNeed);
        if(placeB)
            return true;
        else
            return false;
    }
    return  false;
}


void place(VM& vm, PM& pm, int mode){
    if(!fit(vm,pm,mode)){
        cout<<"Error Place!!!!"<<endl;
        exit(0);
    }
    int cpuNeed = vm.type.cpuNum;
    int memNeed = vm.type.memNum;
    if(mode==2){
        cpuNeed /= 2;
        memNeed /= 2;
        int a = pm.magicA(); singleList[a].erase(pm.p0);
        int b = pm.magicB(); singleList[b].erase(pm.p1);
        int ab = pm.magicAB(); dualList[ab].erase(pm.p2);
        pm.A.first -= cpuNeed;
        pm.A.second -= memNeed;
        pm.B.first -= cpuNeed;
        pm.B.second -= memNeed;
        pm.vmSet[vm.index] = 2;
        vm.placement = make_pair(pm.id, mode);
        a = pm.magicA(); singleList[a].push_back(pm.id); pm.p0 = --singleList[a].end();
        b = pm.magicB(); singleList[b].push_back(pm.id); pm.p1 = --singleList[b].end();
        ab = pm.magicAB(); dualList[ab].push_back(pm.id);  pm.p2 = --dualList[ab].end();
    }
    else if(mode==0){
        int a = pm.magicA(); singleList[a].erase(pm.p0);
        int ab = pm.magicAB(); dualList[ab].erase(pm.p2);
        pm.A.first -= cpuNeed;
        pm.A.second -= memNeed;
        pm.vmSet[vm.index] = 0;
        vm.placement = make_pair(pm.id, mode);
        a = pm.magicA(); singleList[a].push_back(pm.id); pm.p0 = --singleList[a].end();
        ab = pm.magicAB(); dualList[ab].push_back(pm.id);  pm.p2 = --dualList[ab].end();
    }
    else if(mode==1){
        int b = pm.magicB(); singleList[b].erase(pm.p1);
        int ab = pm.magicAB(); dualList[ab].erase(pm.p2);
        pm.B.first -= cpuNeed;
        pm.B.second -= memNeed;
        pm.vmSet[vm.index] = 1;
        vm.placement = make_pair(pm.id, mode);
        b = pm.magicB(); singleList[b].push_back(pm.id); pm.p1 = --singleList[b].end();
        ab = pm.magicAB(); dualList[ab].push_back(pm.id);  pm.p2 = --dualList[ab].end();
    }
}
void unplace(VM& vm){
    if(vm.placement.first==-1){
        cout<<"Error Unplace begin!!!!"<<endl;
        exit(0);
    }
    PM& pm = pms[vm.placement.first];
    int mode = vm.placement.second;
    vm.placement = make_pair(-1,-1);
    int cpuNeed = vm.type.cpuNum;
    int memNeed = vm.type.memNum;
    if(mode==2){
        cpuNeed /= 2;
        memNeed /= 2;
        int a = pm.magicA(); singleList[a].erase(pm.p0);
        int b = pm.magicB(); singleList[b].erase(pm.p1);
        int ab = pm.magicAB(); dualList[ab].erase(pm.p2);
        pm.A.first += cpuNeed;
        pm.A.second += memNeed;
        pm.B.first += cpuNeed;
        pm.B.second += memNeed;
        pm.vmSet.erase(vm.index);
        a = pm.magicA(); singleList[a].push_back(pm.id); pm.p0 = --singleList[a].end();
        b = pm.magicB(); singleList[b].push_back(pm.id); pm.p1 = --singleList[b].end();
        ab = pm.magicAB(); dualList[ab].push_back(pm.id);  pm.p2 = --dualList[ab].end();
    }
    else if(mode==0){
        int a = pm.magicA(); singleList[a].erase(pm.p0);
        int ab = pm.magicAB(); dualList[ab].erase(pm.p2);
        pm.A.first += cpuNeed;
        pm.A.second += memNeed;
        pm.vmSet.erase(vm.index);
        a = pm.magicA(); singleList[a].push_back(pm.id); pm.p0 = --singleList[a].end();
        ab = pm.magicAB(); dualList[ab].push_back(pm.id);  pm.p2 = --dualList[ab].end();
    }
    else if(mode==1){
        int b = pm.magicB(); singleList[b].erase(pm.p1);
        int ab = pm.magicAB(); dualList[ab].erase(pm.p2);
        pm.B.first += cpuNeed;
        pm.B.second += memNeed;
        pm.vmSet.erase(vm.index);
        b = pm.magicB(); singleList[b].push_back(pm.id); pm.p1 = --singleList[b].end();
        ab = pm.magicAB(); dualList[ab].push_back(pm.id);  pm.p2 = --dualList[ab].end();
    }
    if(pm.A.first>pm.type.cpuNum/2||pm.A.second>pm.type.memNum/2||pm.B.first>pm.type.cpuNum/2||pm.B.second>pm.type.memNum/2){
        cout<<"Error Unplace end!!!!"<<endl;
        exit(0);
    }
}
void insert(VM& vm, PM& pm, int mode){
    place(vm, pm, mode);
    cpuSum += vm.type.cpuNum;
    memSum += vm.type.memNum;
    cpuNow += vm.type.cpuNum;
    memNow += vm.type.memNum;
    salesSum += vm.myOffer;
    vmNow ++;

    if(vm.type.cpuNum+vm.type.memNum<totalmin){
        totalmin = vm.type.cpuNum+vm.type.memNum;
        totalmin_cpu = vm.type.cpuNum;
        totalmin_mem = vm.type.memNum;
    }
}
void erase(VM& vm){
    cpuNow -= vm.type.cpuNum;
    memNow -= vm.type.memNum;
    vmNow --;
    unplace(vm);
}
pair<int,int> bestfit(VM& vm){
    int mg = vm.magic();
    if(vm.type.isDual){
        int gap = 40000;
        int bestpm = -1;
        int bestmode = -1;
        for(int i=mg;i<dualListSize;i++){
            if(dualList[i].size()==0) continue;
            for(auto it=dualList[i].begin();it!=dualList[i].end();it++){
                int pmID = (*it);
                PM& pm = pms[pmID];
                if(!fit(vm, pm, 2)) continue;

                int cpuleftA = pm.A.first - vm.type.cpuNum/2;
                int memleftA = pm.A.second - vm.type.memNum/2;
                int cpuleftB = pm.B.first - vm.type.cpuNum/2;
                int memleftB = pm.B.second - vm.type.memNum/2;

                if((cpuleftA+memleftA+cpuleftB+memleftB>uselessspace)&&(cpuleftA<totalmin_cpu/2||memleftA<totalmin_mem/2||cpuleftB<totalmin_cpu/2||memleftB<totalmin_mem/2))
                    continue;

                int nowgap = pm.remain() - vm.type.cpuNum - vm.type.memNum;
                if(nowgap<gap){
                    gap = nowgap;
                    bestpm = pmID;
                    bestmode = 2;
                }
            }
        }
        if(bestpm!=-1)
            return make_pair(bestpm,bestmode);

        for(int i=mg;i<dualListSize;i++){
            if(dualList[i].size()==0) continue;
            for(auto it=dualList[i].begin();it!=dualList[i].end();it++){
                int pmID = (*it);
                PM& pm = pms[pmID];
                if(!fit(vm, pm, 2)) continue;
                int nowgap = pm.remain() - vm.type.cpuNum - vm.type.memNum;
                if(nowgap<gap){
                    gap = nowgap;
                    bestpm = pmID;
                    bestmode = 2;
                }
            }
        }
        if(bestpm!=-1)
            return make_pair(bestpm,bestmode);
    }
    else{
        int gap = 40000;
        int bestpm = -1;
        int bestmode = -1;
        for(int i=mg;i<singleListSize;i++){
            if(singleList[i].size()==0) continue;
            for(auto it=singleList[i].begin();it!=singleList[i].end();it++){
                int pmID = (*it);
                PM& pm = pms[pmID];
                int mode = (pm.p0==it)?0:1;
                if(!fit(vm, pm, mode)) continue;
                int cpuleft,memleft;
                if(mode==0){
                    cpuleft = pm.A.first - vm.type.cpuNum;
                    memleft = pm.A.second - vm.type.memNum;
                }
                else{
                    cpuleft = pm.B.first - vm.type.cpuNum;
                    memleft = pm.B.second - vm.type.memNum;
                }
                ////////
                if((cpuleft+memleft>uselessspace)&&(cpuleft<totalmin_cpu||memleft<totalmin_mem)) continue;
                int nowgap = pm.remain() - vm.type.cpuNum - vm.type.memNum;
                if(nowgap<gap){
                    gap = nowgap;
                    bestpm = pmID;
                    bestmode = mode;
                }
            }
        }
        if(bestpm!=-1)
            return make_pair(bestpm,bestmode);
        for(int i=mg;i<singleListSize;i++){
            if(singleList[i].size()==0) continue;
            for(auto it=singleList[i].begin();it!=singleList[i].end();it++){
                int pmID = (*it);
                PM& pm = pms[pmID];
                int mode = (pm.p0==it)?0:1;
                if(!fit(vm, pm, mode)) continue;
                int nowgap = pm.remain() - vm.type.cpuNum - vm.type.memNum;
                if(nowgap<gap){
                    gap = nowgap;
                    bestpm = pmID;
                    bestmode = mode;
                }
            }
        }
        if(bestpm!=-1)
            return make_pair(bestpm,bestmode);
    }
    return make_pair(-1,-1);
}
pair<int,int> bestfit_border(VM& vm, int border){

    int mg = vm.magic();
    if(vm.type.isDual){
        int gap = 40000;
        int bestpm = -1;
        int bestmode = -1;

        for(int i=mg;i<dualListSize;i++){
            if(dualList[i].size()==0) continue;
            for(auto it=dualList[i].begin();it!=dualList[i].end();it++){
                int pmID = (*it);
                PM& pm = pms[pmID];
                if(pmID<border) continue;
                if(!fit(vm, pm, 2)) continue;
                int nowgap = pm.remain() - vm.type.cpuNum - vm.type.memNum;
                if(nowgap<gap){
                    gap = nowgap;
                    bestpm = pmID;
                    bestmode = 2;
                }
            }
        }
        if(bestpm!=-1)
            return make_pair(bestpm,bestmode);
    }
    else{
        int gap = 40000;
        int bestpm = -1;
        int bestmode = -1;
        for(int i=mg;i<singleListSize;i++){
            if(singleList[i].size()==0) continue;
            for(auto it=singleList[i].begin();it!=singleList[i].end();it++){
                int pmID = (*it);
                PM& pm = pms[pmID];
                int mode = (pm.p0==it)?0:1;
                if(pmID<border) continue;
                if(!fit(vm, pm, mode)) continue;
                int nowgap = pm.remain() - vm.type.cpuNum - vm.type.memNum;
                if(nowgap<gap){
                    gap = nowgap;
                    bestpm = pmID;
                    bestmode = mode;
                }
            }
        }
        if(bestpm!=-1)
            return make_pair(bestpm,bestmode);
    }
    return make_pair(-1,-1);
}

double divergence(vector<double>& p, vector<double>& q){
    vector<double> m;
    for(int i=0;i<p.size();i++)
        m.push_back((p[i]+q[i])/2);
    double pp = 0;
    for(int i=0;i<p.size();i++){
        pp += p[i]*log(p[i]/m[i]);
    }
    double qq = 0;
    for(int i=0;i<q.size();i++){
        qq += q[i]*log(q[i]/m[i]);
    }
    return 0.5*pp + 0.5*qq;
}
double purchaseScore(PMType& pmtype, VM& vm){
    double p0 = pmtype.cpuNum - vm.type.cpuNum;
    double p1 = pmtype.memNum - vm.type.memNum;
    double q0 = cpuNow;
    double q1 = memNow;
    vector<double> p = {p0/(p0+p1), p1/(p0+p1)};
    vector<double> q = {q0/(q0+q1), q1/(q0+q1)};
    double js = divergence(p,q);
    double s1 = js * (T - nowTime) * 100;
    double s2 = (pmtype.purchaseCost*1.0 + pmtype.dailyCost * (T-nowTime) * 1.0)*5.0/(pmtype.cpuNum + pmtype.memNum);
    double s3 = (pmtype.purchaseCost*1.0 + pmtype.dailyCost * (T-nowTime) * 1.0)/150.0;
    return s1 + s2 + s3;
}
unordered_map<string, int> pmtobuy;
PMType& try_purchase(VM& vm){
    bool scored = false;
    double bestscore;
    int bestpmtype = -1;
    for(int i=0;i<pmtypeCnt;i++){
        PMType& pmtype = pmtypes[i];
        if((vm.type.isDual&&fit(vm, pmtype, 2))||(!vm.type.isDual&&fit(vm, pmtype, 0))){
            double score = purchaseScore(pmtype, vm);
            if(!scored || score < bestscore){
                scored = true;
                bestscore = score;
                bestpmtype = i;
            }
        }
    }
    return pmtypes[bestpmtype];
}
PM& purchase(VM& vm){
    bool scored = false;
    double bestscore;
    int bestpmtype = -1;
    for(int i=0;i<pmtypeCnt;i++){
        PMType& pmtype = pmtypes[i];
        if((vm.type.isDual&&fit(vm, pmtype, 2))||(!vm.type.isDual&&fit(vm, pmtype, 0))){
            double score = purchaseScore(pmtype, vm);
            if(!scored || score < bestscore){
                scored = true;
                bestscore = score;
                bestpmtype = i;
            }
        }
    }
    PMType& pmtype = pmtypes[bestpmtype];
    purchaseCostSum += pmtype.purchaseCost;
    cpuBuy += pmtype.cpuNum;
    memBuy += pmtype.memNum;
    pms[pmCnt] = PM(pmCnt, pmtype, nowTime);
    pmIDs[pmCnt] = pmCnt;
    pmCnt ++;
    pmtobuy[pmtype.name] ++;

    PM& pm = pms[pmCnt-1];


    int a = pm.magicA(); singleList[a].push_back(pm.id); pm.p0 = --singleList[a].end();
    int b = pm.magicB(); singleList[b].push_back(pm.id); pm.p1 = --singleList[b].end();
    int ab = pm.magicAB(); dualList[ab].push_back(pm.id); pm.p2 = --dualList[ab].end();
    
    return pms[pmCnt-1];
}
int migrateMax = 0;
int migrateCnt = 0;
vector<string> migratePrint;
pair<int,int> bestfit_migrate(VM& vm){
    int mg = vm.magic();
    int block = 2000;
    if(vm.type.isDual){
        for(int i=mg;i<dualListSize;i++){
            if(i-mg>=block) break;
            if(dualList[i].size()==0) continue;
            for(auto it=dualList[i].begin();it!=dualList[i].end();it++){
                int pmID = (*it);
                PM& pm = pms[pmID];
                if(pm.isEmpty()) continue;
                if(!fit(vm, pm, 2))continue;
                int cpuleftA = pm.A.first - vm.type.cpuNum/2;
                int memleftA = pm.A.second - vm.type.memNum/2;
                int cpuleftB = pm.B.first - vm.type.cpuNum/2;
                int memleftB = pm.B.second - vm.type.memNum/2;
            
                if((cpuleftA+memleftA+cpuleftB+memleftB>uselessspace)&&(cpuleftA<totalmin_cpu/2||memleftA<totalmin_mem/2||cpuleftB<totalmin_cpu/2||memleftB<totalmin_mem/2))
                    continue;

                return make_pair(pmID, 2);
            }
        }
    }
    else{
        for(int i=mg;i<singleListSize;i++){
            if(i-mg>=block) break;
            if(singleList[i].size()==0) continue;
            for(auto it=singleList[i].begin();it!=singleList[i].end();it++){
                int pmID = (*it);
                PM& pm = pms[pmID];
                int mode = (pm.p0==it)?0:1;
                if(pm.isEmpty()) continue;
                if(!fit(vm, pm, mode)) continue;
                int cpuleft,memleft;
                if(mode==0){
                    cpuleft = pm.A.first - vm.type.cpuNum;
                    memleft = pm.A.second - vm.type.memNum;
                }
                else{
                    cpuleft = pm.B.first - vm.type.cpuNum;
                    memleft = pm.B.second - vm.type.memNum;
                }
                ////////
                if((cpuleft+memleft>uselessspace)&&(cpuleft<totalmin_cpu||memleft<totalmin_mem))
                    continue;
                return make_pair(pmID, mode);
            }
        }
    }
    return make_pair(-1,-1);
}
int continues_delete_day = 0;

bool chance = true;
int migrationmax = 0;
int migrationcnt = 0;
void migrate(){
    int limit = vmNow*3/100;
    
    if(chance && nowTime > T/2 && continues_delete_day > 10){
        limit = vmNow;
        chance = false;
    }
    int tmp_limit = limit;
    migrationmax += limit;

    int limit_max = limit;
    int reserved = 30;
    migratePrint.clear();
    migrateMax += limit_max;
    

    for(int k=0;k<3;k++){
    sort(pmIDs,pmIDs+pmCnt,[](int a, int b){
        return pms[a].ratio() > pms[b].ratio();
    });
    
    for(int i1=pmCnt-1;i1>=0;i1--){
        int i = pmIDs[i1];
        if(limit<=reserved) break;
        if(pms[i].magicAB()==0) continue;

        vector<int> vmtomigrate;
        PM& src = pms[i];
        for(auto it: src.vmSet){
            vmtomigrate.push_back(it.first);
        }
        for(int vmIndex: vmtomigrate){
            if(limit<=reserved) break;
            VM& vm = vms[vmIndex];
            pair<int, int> old_placement = vm.placement;
            unplace(vm);
            pair<int, int> placement = bestfit_migrate(vm);
            if(placement==old_placement||placement.first==-1){
                place(vm, pms[old_placement.first], old_placement.second);
                continue;
            }
            limit--;
            place(vm, pms[placement.first], placement.second);
            if(placement.second==2)
                migratePrint.push_back("("+to_string(vm.id)+", "+to_string(pmIDmap[placement.first])+")");
            else
                migratePrint.push_back("("+to_string(vm.id)+", "+to_string(pmIDmap[placement.first])+", "+(placement.second?"B":"A")+")");
        }
    }
    }


    unordered_map<string, vector<int> > combine;
    combine.clear();
    for(int i=0;i<pmCnt;i++){
        PM& pm = pms[i];
        if(pm.isEmpty()) continue;
        if(pm.isEmptyA()||pm.isEmptyB()){
            if(combine.find(pm.type.name)==combine.end()){
                vector<int> tmp;
                tmp.push_back(pm.id);
                combine[pm.type.name] = tmp;
            }
            else{
                combine[pm.type.name].push_back(pm.id);
            }
        }
    }

    
    for(auto it: combine){
        if(limit<=0) break;
        vector<int>& ls = it.second;
        for(int i=0;i*2+1<ls.size();i++){
            if(limit<=0) break;
            int pm1 = ls[2*i];
            int pm2 = ls[2*i+1];
            int mode2 = pms[pm2].isEmptyA()?0:1;
            vector<int> vmtomigrate;
            for(auto itt: pms[pm1].vmSet)
                vmtomigrate.push_back(itt.first);
            for(int vmIndex: vmtomigrate){
                if(limit<=0) break;
                VM& vm = vms[vmIndex];
                unplace(vm);
                limit--;
                place(vm, pms[pm2], mode2);
                migratePrint.push_back("("+to_string(vm.id)+", "+to_string(pmIDmap[pm2])+", "+(mode2?"B":"A")+")");
            }
        }
    }
    

    vector<int> rest;
    vector<bool> vis;
    for(int i=0;i<pmCnt;i++){
        PM& pm = pms[i];
        if(pm.isEmpty()) continue;
        if(pm.isEmptyA()||pm.isEmptyB()){
            rest.push_back(i);
            vis.push_back(false);
        }
    }
    sort(rest.begin(),rest.end(),[](int a,int b){
        return pms[a].type.cpuNum + pms[a].type.memNum > pms[b].type.cpuNum + pms[b].type.memNum;
    });
    for(int i=0;i<rest.size();i++){
        if(limit<=0) break;
        if(vis[i]) continue;
        for(int j=i+1;j<rest.size();j++){
            if(limit<=0) break;
            if(vis[j]) continue;
            PM& pm1 = pms[rest[i]];
            PM& pm2 = pms[rest[j]];
            int mode1 = pm1.isEmptyA()?0:1;
            if((pm2.type.cpuNum-pm2.A.first-pm2.B.first<=pm1.type.cpuNum/2)&&(pm2.type.memNum-pm2.A.second-pm2.B.second<=pm1.type.memNum/2)){
                vis[i]=vis[j]=true;
                vector<int> vmtomigrate;
                for(auto itt: pm2.vmSet){
                    vmtomigrate.push_back(itt.first);
                  
                }
               
                
                for(int vmIndex: vmtomigrate){
                    if(limit<=0) break;
                    VM& vm = vms[vmIndex];
                    unplace(vm);
                    limit--;
                    place(vm, pm1, mode1);
                    migratePrint.push_back("("+to_string(vm.id)+", "+to_string(pmIDmap[pm1.id])+", "+(mode1?"B":"A")+")");
                }
                break;
            }
        }
    }
    migrationcnt += tmp_limit - limit;
}

vector<int> oppProfit;
vector<double> oppRate;
double off = 0.6;
double obtainRate = 0.6;
double fenbu[10] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
double level = 0.999;
vector<pair<int,int> > bindvec;
void sendBidding(int t){
    /*
    if(t<=T-2 && (requestOffset[t+2]-requestOffset[t+1])*1.0/(requestOffset[t+1]-requestOffset[t])>20){
        for(int r=requestOffset[t];r<requestOffset[t+1];r++){
            if(requests[r].type==1){
                VM& vm = vms[vmMap[requests[r].id]];
                vm.value = vm.baseOffer * 0.98;
                vm.myOffer = vm.baseOffer * 0.98;
                print(to_string(vm.myOffer));
            }
        }
        return;
    }*/

    vector<pair<int,int> > deleteSum;
    deleteSum.resize(requestOffset[t+1]-requestOffset[t]);
    for(int r=requestOffset[t];r<requestOffset[t+1];r++){
        if(r==requestOffset[t]){
            if(requests[r].type==1){
                VM& vm = vms[vmMap[requests[r].id]];
                deleteSum[0] = make_pair(-vm.type.cpuNum, -vm.type.memNum);
            }
            else{
                VM& vm = vms[vmMap[requests[r].id]];
                if(vm.obtain==true)
                    deleteSum[0] = make_pair(vm.type.cpuNum, vm.type.memNum);
                else
                    deleteSum[0] = make_pair(0,0);
            }
        }
        else{
            pair<int,int>& previous = deleteSum[r-requestOffset[t]-1];
            if(requests[r].type==1){
                VM& vm = vms[vmMap[requests[r].id]];
                deleteSum[r-requestOffset[t]] = make_pair(previous.first - vm.type.cpuNum, previous.second - vm.type.memNum);
            }
            else{
                VM& vm = vms[vmMap[requests[r].id]];
                if(vm.obtain==true)
                    deleteSum[r-requestOffset[t]] = make_pair(previous.first + vm.type.cpuNum, previous.second + vm.type.memNum);
                else
                    deleteSum[r-requestOffset[t]] = previous;
            }
        }
    }

    vector<pair<int,int> > indexpairs;
    for(int r=requestOffset[t];r<requestOffset[t+1];r++){
        if(requests[r].type==1)
            indexpairs.push_back(make_pair(vmMap[requests[r].id], r-requestOffset[t]));
    }
    sort(indexpairs.begin(), indexpairs.end(), [](pair<int,int>& a, pair<int,int>& b){
        VM& vma = vms[a.first];
        VM& vmb = vms[b.first];
        if( vma.baseOffer*1.0/(vma.type.cpuNum+vma.type.memNum)*vma.lifeCycle == vmb.baseOffer*1.0/(vmb.type.cpuNum+vmb.type.memNum)*vmb.lifeCycle )
            return a.second < b.second;
        else
            return vma.baseOffer*1.0/(vma.type.cpuNum+vma.type.memNum)*vma.lifeCycle > vmb.baseOffer*1.0/(vmb.type.cpuNum+vmb.type.memNum)*vmb.lifeCycle;
    });

    int cpuLeft = 0;
    int memLeft = 0;
    for(int i=0;i<pmCnt;i++){
        cpuLeft += pms[i].A.first;
        cpuLeft += pms[i].B.first;
        memLeft += pms[i].A.second;
        memLeft += pms[i].B.second;
    }


    for(auto indexpair: indexpairs){
        VM& vm = vms[indexpair.first];
        int cpuLeftNow = cpuLeft + deleteSum[indexpair.second].first;
        int memLeftNow = memLeft + deleteSum[indexpair.second].second;
        int cpuNeed = vm.type.cpuNum;
        int memNeed = vm.type.memNum;
        pair<int,int> placement = bestfit(vm);
        if(cpuLeftNow >= cpuNeed && memLeftNow >= memNeed){
            if(placement.first!=-1){
                PM& pm = pms[placement.first];
                double use = (vm.type.cpuNum + vm.type.memNum)*1.0/(pm.type.cpuNum+pm.type.memNum);
                vm.cost = pm.type.purchaseCost*1.0/(T-pm.purchaseTime)*use*min(vm.lifeCycle, T-nowTime) + pm.type.dailyCost * use * min(vm.lifeCycle, T-nowTime);
            }
            else{
                PM& pm = pms[vm.id%pmCnt];
                double use = (vm.type.cpuNum + vm.type.memNum)*1.0/(pm.type.cpuNum+pm.type.memNum);
                vm.cost = pm.type.purchaseCost*1.0/(T-pm.purchaseTime)*use*min(vm.lifeCycle, T-nowTime)+ pm.type.dailyCost * use * min(vm.lifeCycle, T-nowTime);
            }
        }
        else{
            PMType& pmtype = try_purchase(vm);
            double use = (vm.type.cpuNum + vm.type.memNum)*1.0/(pmtype.cpuNum+pmtype.memNum);
            vm.cost = pmtype.purchaseCost*1.0/(T-nowTime)*use*min(vm.lifeCycle, T-nowTime) + pmtype.dailyCost * use * min(vm.lifeCycle, T-nowTime);
        }

        

        if(t<T/100){
            vm.myOffer = vm.cost;
            vm.myOffer = min(vm.myOffer, vm.baseOffer);
            vm.myOffer = max(vm.myOffer, 0);
            continue;
        }

        if(obtainRate < 0.5){
            double down = 1;
            double up = 0.9;
            for(int i=t-1;i>=max(0,t-3);i--){
                down += oppProfit[i];
                up += oppProfit[i]*oppRate[i];
            }
            off = up/down;
        }
        else{
            off = off + 0.01;
        }
        int oppOffer = vm.baseOffer * off;
        if(oppOffer < vm.cost){
            vm.myOffer = -1;
        }
        else if(oppOffer*0.85 < vm.cost){
            vm.myOffer = oppOffer;
            vm.myOffer = min(vm.myOffer, vm.baseOffer);
            vm.myOffer = max(vm.myOffer, 0);
        }
        else{
            vm.myOffer = oppOffer * 0.85;
            vm.myOffer = min(vm.myOffer, vm.baseOffer);
            vm.myOffer = max(vm.myOffer, 0);
        }
        
        if(vm.myOffer!=-1){
            cpuLeft -= cpuNeed;
            memLeft -= memNeed;
        }
    }

    
    bindvec.clear();
    for(int r=requestOffset[t];r<requestOffset[t+1];r++){
        if(requests[r].type==1 && vms[vmMap[requests[r].id]].myOffer!=-1){
            VM& vm = vms[vmMap[requests[r].id]];
            bindvec.push_back(make_pair( vm.cost - vm.baseOffer, vm.index));
        }
    }
    sort(bindvec.begin(), bindvec.end());
    
    for(int i=0;i<bindvec.size();i++){
        pair<int, int>& pa = bindvec[i];;
        vms[pa.second].bindindex = i;
    }
    /*
    for(int i=0;i<min(3, (int)bindvec.size()); i++){
        VM& vm = vms[bindvec[i].second];
        vm.bind = true;
    }*/
    vector<pair<double, int> > newpair;
    for(int i=0;i<min(10, (int)bindvec.size()); i++){
        VM& vm = vms[bindvec[i].second];
        newpair.push_back(make_pair(-fenbu[i], i));
    }
    sort(newpair.begin(), newpair.end());
    int bind_cnt = 3;
    if(level > 0.8){
        for(int i=0;i<min(3, (int)newpair.size()); i++){
            VM& vm = vms[bindvec[newpair[i].second].second];
            vm.myOffer = vm.baseOffer * level * 0.99;
            vm.myOffer = min(vm.myOffer, vm.baseOffer);
            vm.myOffer = max(vm.myOffer, 0);
            vm.bind = true;
            bind_cnt--;
        }
    }
    else{
        int a[3]  = {t%7, t%7+1, t%7+2};
        for(int i=0;i<3;i++){
            if(a[i]<bindvec.size()){
                VM& vm = vms[bindvec[a[i]].second];
                vm.myOffer = vm.baseOffer;
                vm.bind = true;
                bind_cnt--;
            }
        }
    }


    for(int r=requestOffset[t];r<requestOffset[t+1];r++){
        if(requests[r].type==1){
            VM& vm = vms[vmMap[requests[r].id]];
            if(vm.myOffer!=-1){
                vm.myOffer = min(vm.myOffer, vm.baseOffer);
                vm.myOffer = max(vm.myOffer, 0);
            }
            if(vm.bind)
                print(to_string(vm.myOffer)+", #");
            else
                print(to_string(vm.myOffer));
        }
    }
}
void recvBidding(int t){
#ifndef OFFLINE
    fflush(stdout);

    int obtainCnt = 0;
    int totalCnt = 0;

    int opp_profit_sum = 0;
    double opp_rate_sum = 0.0;
    int opp_cnt = 0;

    double opp_bind_rate_sum = 0.0;
    int opp_bind_sum = 0;
    for(int r=requestOffset[t];r<requestOffset[t+1];r++){
        if(requests[r].type==1){
            char buffer[20];
            gets(buffer);
            bool obtain = buffer[1]=='1';
            char* pos = buffer+4;
            int othersOffer = 0;
            int fu = 1;
            if(*pos=='-'){
                othersOffer = -1;
            }
            else{
                while(*pos!=')'){
                    othersOffer = othersOffer*10 + *pos - '0';
                    pos++;
                }
            }
            
            VM& vm = vms[vmMap[requests[r].id]];
            vm.othersOffer = othersOffer;

            totalCnt ++;

            if(vm.myOffer!=-1 && vm.myOffer < vm.othersOffer && vm.obtain==false){
                if(vm.bindindex!=-1&&vm.bindindex<10)
                    fenbu[vm.bindindex]++;
                opp_bind_rate_sum += vm.othersOffer*1.0/vm.baseOffer;
                opp_bind_sum ++;
            }

            if(!obtain){
                vm.obtain = false;
            }
            else{
                obtainCnt ++;
                
            }
            if(othersOffer!=-1){
                opp_profit_sum += othersOffer;
                opp_rate_sum += othersOffer*1.0/vm.baseOffer;
                opp_cnt ++;
            }
        }
    }

    double new_level = opp_bind_sum == 0 ? 1 : opp_bind_rate_sum/opp_bind_sum;
    level = level*0.5 + new_level*0.5;

    obtainRate = totalCnt==0 ? 0.5 : obtainCnt*1.0/totalCnt;
    oppProfit.push_back(opp_profit_sum+1);
    oppRate.push_back(opp_cnt==0?1:opp_rate_sum/opp_cnt);
#endif
}

void printOneDay(int t){
    print("(purchase, "+to_string(pmtobuy.size())+")");
    unordered_map<string,int> startIndex;
    int index = 0;
    for(unordered_map<string, int>::iterator it = pmtobuy.begin();it!=pmtobuy.end();it++){
        print("("+it->first+", "+to_string(it->second)+")");
        startIndex[it->first] = index;
        index += it->second;
    }
    for(int i=0;i<index;i++){
        PM& pm = pms[pmCnt-i-1];
        pmIDmap[pm.id] = pmCnt - index + startIndex[pm.type.name];
        startIndex[pm.type.name]++;
    }
    pmtobuy.clear();
    startIndex.clear();
    print("(migration, "+to_string(migratePrint.size())+")");
    for(string& line: migratePrint)
        print(line);
    migratePrint.clear();
    for(int r=requestOffset[t];r<requestOffset[t+1];r++){
        if(requests[r].type==0) continue;
        VM& vm = vms[vmMap[requests[r].id]];
        if(vm.obtain==false) continue;
        int pmID = pmIDmap[vm.placement.first];
        int mode = vm.placement.second;
        if(mode==2)
            print("("+to_string(pmID)+")");
        else if(mode==0)
            print("("+to_string(pmID)+", A)");
        else if(mode==1)
            print("("+to_string(pmID)+", B)");
    }
}


void solve(){
    double total_time_cost = 0.0;
    for(int t=0;t<T;t++){
        nowTime = t;
        if(total_time_cost < 80){
            timeval t_start, t_end;
            gettimeofday( &t_start, NULL);
            migrate();
            gettimeofday( &t_end, NULL);
            double delta_t = (t_end.tv_sec-t_start.tv_sec) + (t_end.tv_usec-t_start.tv_usec)/1000000.0;
            total_time_cost += delta_t;
        }
        sendBidding(t);
        recvBidding(t);
        
        int delete_today = 0;
        for(int r=requestOffset[t];r<requestOffset[t+1];r++){
            VM& vm = vms[vmMap[requests[r].id]];
            if(requests[r].type==0 && vm.obtain==1){
                delete_today++;
            }
        }
        if(delete_today>80)
            continues_delete_day ++;
        else
            continues_delete_day = 0;
        

        vector<int> wait;
        int now = requestOffset[t];
        while(now<requestOffset[t+1]){
            vector<int> addrequests;
            while(now<requestOffset[t+1]&&requests[now].type==1){
                if(vms[vmMap[requests[now].id]].obtain==true){
                    addrequests.push_back(vmMap[requests[now].id]);
                };
                now++;
            }
            if(addrequests.size()>0){
                sort(addrequests.begin(),addrequests.end(),[](int a, int b){
                    if(vms[a].addTime + vms[a].lifeCycle == vms[b].addTime + vms[b].lifeCycle)
                        return vms[a].type.cpuNum + vms[a].type.memNum > vms[b].type.cpuNum + vms[b].type.memNum;
                    return vms[a].addTime + vms[a].lifeCycle > vms[b].addTime + vms[b].lifeCycle;
                });
            }
            for(int vmIndex: addrequests){
                VM& vm = vms[vmIndex];
                pair<int, int> placement = bestfit(vm);
                if(placement.first==-1&&vm.lifeCycle!=0){
                    //放不下且不是当天删除
                    wait.push_back(vmIndex);
                }
                else{
                    if(placement.first==-1){
                        //放不下且当天删除
                        PM& newpm = purchase(vm);
                        insert(vm, newpm, vm.type.isDual?2:0);
                    }
                    else{
                        //放得下
                        PM& oldpm = pms[placement.first];
                        insert(vm, oldpm, placement.second);
                    }
                }
            }
            while(now<requestOffset[t+1]&&requests[now].type==0){
                VM& vm =  vms[vmMap[requests[now].id]];
                if(vm.obtain==true){
                    erase(vm);
                }
                now++;
            }
        }
        sort(wait.begin(),wait.end(),[](int a, int b){
            if(vms[a].addTime + vms[a].lifeCycle == vms[b].addTime + vms[b].lifeCycle)
                return vms[a].type.cpuNum + vms[a].type.memNum > vms[b].type.cpuNum + vms[b].type.memNum;
            return vms[a].addTime + vms[a].lifeCycle > vms[b].addTime + vms[b].lifeCycle;
        });
        int pmBorder = pmCnt;
        for(int vmIndex: wait){
            VM& vm = vms[vmIndex];
            pair<int,int> placement = bestfit_border(vm, pmBorder);
            if(placement.first==-1){
                PM& newpm = purchase(vm);
                insert(vm, newpm, vm.type.isDual?2:0);
            }
            else{
                PM& oldpm = pms[placement.first];
                insert(vm, oldpm, placement.second);
            }
        }

        printOneDay(t);
        if(K+t<T)
            readOneDay(K+t);

       

#ifdef OFFLINE
        int pmUse = 0;
        int cpuUsePM = 0, memUsePM = 0;
        for(int i=0;i<pmCnt;i++){
            if(pms[i].vmSet.empty()) continue;
            pmUse ++;
            dailyCostSum += ll(pms[i].type.dailyCost);
            cpuUsePM += pms[i].type.cpuNum;
            memUsePM += pms[i].type.memNum;
        }
        if(t%100==0||t==T-1){
            LOG_INFO("Time:%d  VM:%d  PM:%d/%d  Sales: %lld  Cost:%lld  Profit:%lld",t,vmNow,pmUse,pmCnt,salesSum, purchaseCostSum+dailyCostSum,salesSum-(purchaseCostSum+dailyCostSum) );
        }
        if(t==T-1)
            LOG_INFO("migration:%d/%d",migrationcnt,migrationmax);
#endif
    }

}

int main(){
    read();
    solve();
}
