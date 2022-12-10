#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

using namespace std;
const char ACTION_BUY='B';
const char ACTION_SELL='S';
const char ACTION_MODIFY='M';
const char ACTION_CANCEL='C';
const char ACTION_PRINT='P';

const char ORDERTYPE_IOC='I';
const char ORDERTYPE_GFD='G';

const char MATCH_FILLED='F';
const char MATCH_CXLD='X';
const char MATCH_UNFILLED='U'; //PARTIAL or NONE 
#define MAX_ORDER_LEN 250

class Order{
  public:
    string orderId;
    char action;
    char orderType;
    float price;
    int qty;
    time_t orderTime; //since epoch
    
    void create(string id, char a, char o, float p, int q){
        action=a;
        orderType=o;
        price=p;
        qty=q;
        orderId=id;
        time(&orderTime);
    }
    void clean(){
        orderId = "";
        action='\0';
        orderType='\0';
        price=0.0;
        qty=0;
        orderTime=0;
    }
    void print(){     
        cout<<"OrderID:"<<orderId<<" Action:"<<action<<" Type:"<<orderType<<" Price:"<<price<<" Qty:"<<qty<<" Time:"<<orderTime<<endl;
    }
    void printShort(){
        cout<<" Price:"<<price<<" Qty:"<<qty<<endl;       
    }

    bool match(Order *o){
         if (action == ACTION_BUY){
             return (price>=o->price)?true:false;
         }else{
             return (price<=o->price)?true:false;
         }
    }
    
};

class Book{
   public:
    list<Order*> sellOrders;
    list<Order*> buyOrders;
    list<Order*>freeOrders;
    map<string, Order*> allOrders;
    int seq;
    Book(){
        seq=1;
    }
    void printTrade(const Order &o, const Order* bo, int filled){
        if (o.action==ACTION_BUY){
            cout<<"TRADE "<<o.orderId<<" "<<o.price<<" "<<filled<<" "<<bo->orderId<<" "<<
            bo->price<<" "<<filled<<endl;
        }else{
            cout<<"TRADE "<<bo->orderId<<" "<<bo->price<<" "<<filled<<" "<<o.orderId<<" "<<
            o.price<<" "<<filled<<endl;
        }
    }
    void printFullBook(){
        cout<<"===Full Book ============="<<endl;
        for (auto it=sellOrders.begin();it!=sellOrders.end();++it){
            cout<<(*it)->orderId<<" "<<(*it)->price<<" "<<(*it)->qty<<endl;
        }
        for (auto it=buyOrders.begin();it!=buyOrders.end();++it){
            cout<<(*it)->orderId<<" "<<(*it)->price<<" "<<(*it)->qty<<endl;
        }
    }
    void printBook(char action){
        list<Order*> *orderList;
        if (action == ACTION_BUY){
            cout<<"BUY:"<<endl;
            orderList = &buyOrders;
        }else{
            cout<<"SELL:"<<endl;   
            orderList = &sellOrders;  
        }
        float prevPrice=0.0;
        int totQty=0;
        for (auto it=orderList->rbegin(); it!=orderList->rend();it++){
            if (prevPrice==(*it)->price){
                totQty +=(*it)->qty;
            }else{
                if (totQty>0){
                    cout<<prevPrice<<" "<<totQty<<endl;
                }
                totQty = (*it)->qty;
            }
            prevPrice = (*it)->price;
        }
        if (totQty>0){
            cout<<prevPrice<<" "<<totQty<<endl;
        }
    }
    void addToBook(list<Order*>*orderList, Order* o){
        std::list<Order*>::iterator it;
        for (it=orderList->begin(); it != orderList->end();it++){
            if ((*it)->price > o->price){
                break;
            }
        }
        //Add to the head 
        if (it==orderList->begin()){
            orderList->push_front(o);
        }else if (it == orderList->end()) {//add to the end
            orderList->push_back(o);
        }else{// add before it
            orderList->insert(it,o);
        }
        allOrders.insert(std::pair<string,Order *>(o->orderId,o));
    }

    char match(Order &o){
        list<Order*>*orderList;
        int leaveQty=o.qty, filled=0;
        list<Order*>::iterator itl;
        static list<list<Order*>::iterator>toFree;

        if (o.action == ACTION_BUY){
            orderList = &sellOrders;
        }else{
            orderList = &buyOrders;
        }
        for (itl=orderList->begin();itl!=orderList->end() && leaveQty>0;){
            if (o.match((*itl))){
                if ((*itl)->qty > leaveQty){  
                    filled=leaveQty;  
                    (*itl)->qty -= filled;
                    leaveQty=0;
                    printTrade(o,(*itl),filled);
                    break;
                }else{
                    filled=(*itl)->qty;
                    leaveQty -=filled;
                    printTrade(o,(*itl),filled);
                    (*itl)->qty=0;
                    freeOrders.push_back(*itl);
                    (*itl)->clean();
                    itl=orderList->erase(itl);
                }
            }else{
                itl++;
            }
        }

        o.qty = leaveQty;        
        if (o.orderType == ORDERTYPE_IOC){
            return (leaveQty==0?MATCH_FILLED:MATCH_CXLD);
        }else{
            if (leaveQty>0){
                return MATCH_UNFILLED;
            }else{
                return MATCH_FILLED;
            }
        }
    }

    void process(string orderId, char action, char orderType, float price, int qty){
        Order *o=NULL;
        
        if (freeOrders.empty()){
            o=new Order();
        }else{
            o=freeOrders.front();
            freeOrders.pop_front();
        }
        o->create(orderId, action,orderType,price,qty);
        list<Order*> *orderList;
        
        if (o->action == ACTION_BUY){
            orderList = &buyOrders;
        }else{
            orderList = &sellOrders;                
        }
        char ret = match(*o);
        if (ret == MATCH_UNFILLED){
            addToBook(orderList,o);
        }else if (ret == MATCH_CXLD){
            o->clean();
            freeOrders.push_back(o);
            cout<<"IOC order could not be fully filled. Cancelled"<<endl;
        }else{
            cout<<"Order fully filled"<<endl;
            o->clean();
            freeOrders.push_back(o);
        }
    }
    bool cancel(string orderId){

        std::map<string,Order*>::iterator it;
        std::list<Order*>::iterator itl;

        it = allOrders.find(orderId);

        if (it == allOrders.end()){
            cout<<"ERROR "<<orderId<<" Not found. Ignored."<<endl;
            return false;
        }
        Order *o = allOrders[orderId];
        list<Order *>  *orderList;
        if (o->action == ACTION_BUY){
            orderList = &buyOrders;
        }else{
            orderList = &sellOrders;                
        }
        for (itl=orderList->begin();itl != orderList->end();itl++){
            if  ((*itl)->orderId == o->orderId){
                orderList->erase(itl);
                break;
            }
        }
        allOrders.erase(it);
        cout<<"Cancelled Order :";
        o->print();
        o->clean();
        freeOrders.push_back(o);
        return true;
    }
    void replace(string orderId,char action, char orderType,float price, int qty){
        if (cancel(orderId)){
            cout<<"Replaced order :"<<orderId<<endl;
            process(orderId,action,orderType,price,qty);
        }
    }
};
int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    bool done=false;
    Book book;
    char order[MAX_ORDER_LEN]={0};
    vector<string> tokens;
    string tmp;
    string orderRec;
    char action, replaceAction, orderType, mAction;
    float price=0.0;
    int qty=0,i=0;
    string orderId;
    while (!done){
        cout<<"--------------------------------------------------"<<endl;
        cout<<"Enter order as follows"<<endl;
        cout<<"BUY/SELL IOC/GFD price quantity orderID"<<endl;
        cout<<"MODIFY orderID BUY/SELL IOC/GFD price quantity"<<endl;
        cout<<"CANCEL orderID"<<endl;
        cout<<"PRINT"<<endl;
        cout<<"EXIT"<<endl;
        cout<<"--------------------------------------------------"<<endl;
        cout<<"Enter Order :";
        cin.getline(order,MAX_ORDER_LEN);
        orderRec=(const char*)order;
        stringstream st(orderRec);
        getline(st,tmp,' ');
        
        if (tmp=="BUY"){
            action =ACTION_BUY;
        }else if (tmp == "SELL"){
            action=ACTION_SELL;
        }else if (tmp =="MODIFY"){
            action = ACTION_MODIFY;
        }else if (tmp =="CANCEL"){
            action = ACTION_CANCEL;
        }else if (tmp == "PRINT"){
            action=ACTION_PRINT;
        }else if (tmp == "EXIT"){
            exit(0);
        }else{
            cout<<"UNKNOWN First column. Please retry"<<endl;
            continue;
        }
        tokens.clear();

        while(getline(st,tmp,' ')){
            tokens.push_back(tmp);
        }
        if (action==ACTION_MODIFY){
            if (tokens.size() < 5){
                cout<<"Insufficient data to MODIFY order"<<endl;
                continue;
            }
            if (tokens[1] == "BUY")
            {
                mAction=ACTION_BUY;
            }else if (tokens[1] == "SELL"){
                mAction=ACTION_SELL;
            }else{
                cout<<"Unknwon Action in MODIFY!. Please reenter"<<endl;
                continue;
            }
            i=2;
        }
        if (action==ACTION_MODIFY || action == ACTION_CANCEL){
            orderId=tokens[0];
        }

        if (action==ACTION_BUY || action==ACTION_SELL ){
            if (tokens.size() < 4){
                cout<<"Insufficient Data to BUY/SELL order"<<endl;
                continue;
            }
            orderId=tokens[3];
            i=0;
        }
        if (action==ACTION_BUY || action==ACTION_SELL || action==ACTION_MODIFY ){
            if (tokens[i] == "IOC"){
                orderType = ORDERTYPE_IOC;
            }else if (tokens[i] == "GFD"){
                orderType = ORDERTYPE_GFD;
            }else{
                cout<<"BAD order type"<<endl;
                continue;
            }
            price=atof(tokens[i+1].c_str());
            qty = atoi(tokens[i+2].c_str());

            if (price <= 0){
                cout<<"ERROR Price cannot be negative"<<endl;
                continue;
            }
            if (qty <=0){
                cout<<"ERROR Quantity cannot be negative"<<endl;
                continue;
            }
        }
        if (action != ACTION_PRINT){
            if (orderId.length()==0){
                cout<<"ERROR OrderId cannot be blank"<<endl;
                continue;
            }
        }

        switch(action){
            case ACTION_BUY:
            case ACTION_SELL:
                book.process(orderId,action,orderType,price,qty);
                break;
            case ACTION_MODIFY:
                book.replace(orderId,mAction,orderType,price,qty);
                break;
            case ACTION_CANCEL:
                book.cancel(orderId);
                break;
            case ACTION_PRINT:
                book.printBook(ACTION_SELL);
                book.printBook(ACTION_BUY);
                break;
            default:
                cout<<"Unknown choice"<<endl;
                break;
        }                          
    }
    
    return 0;
}