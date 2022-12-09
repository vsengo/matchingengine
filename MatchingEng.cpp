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
const char MATCH_UNFILLED='U';
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
    bool match(Order &o){
         if (action == ACTION_BUY){
             return (price>=o.price)?true:false;
         }else{
             return (price<=o.price)?true:false;
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
    void print(){
        cout<<"Printing Book - Sell Orders"<<endl;
        for (auto it = sellOrders.begin(); it != sellOrders.end();it++){
            (*it)->print();
        }
        cout<<"Printing Book - Buy Orders"<<endl;
        for (auto it = buyOrders.begin(); it != buyOrders.end();it++){
            (*it)->print();
        }
    }
    Order * process(string orderId, char action, char orderType, float price, int qty){
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
        orderList->push_back(o);
        allOrders.insert(std::pair<string,Order *>(o->orderId,o));

        o->print();
        return o;
    }
    void cancel(string orderId){

        std::map<string,Order*>::iterator it;
        std::list<Order*>::iterator itl;

        it = allOrders.find(orderId);

        if (it != allOrders.end()){
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
        }
    }
    void replace(string orderId,char action, char orderType,float price, int qty){
        cancel(orderId);
        Order *o=process(orderId,action,orderType,price,qty);
        cout<<"Replaced order :";
        o->print();
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
    char action, replaceAction, orderType;
    float price=0.0;
    int qty=0,i=0;
    string orderId;
    while (!done){
        cout<<"Enter order as follows"<<endl;
        cout<<"e.g BUY/SELL IOC/GFD price quantity orderID"<<endl;
        cout<<"e.g CANCEL orderID"<<endl;
        cout<<"e.g MODIFY orderID BUY/SELL IOC/GFD price quantity"<<endl;
        cout<<"e.g PRINT"<<endl;
        cout<<"EXIT"<<endl;
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
                cout<<"Insufficient data for MODIFY order"<<endl;
                continue;
            }
            i=1;
        }
        if (action==ACTION_MODIFY || action == ACTION_CANCEL){
            orderId=tokens[0];
        }

        if (action==ACTION_BUY || action==ACTION_SELL ){
            if (tokens.size() < 4){
                cout<<"Insufficient Data for BUY/SELL order"<<endl;
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
        }
   
        switch(action){
            case ACTION_BUY:
            case ACTION_SELL:
                book.process(orderId,action,orderType,price,qty);
                break;
            case ACTION_MODIFY:
                book.replace(orderId,action,orderType,price,qty);
                break;
            case ACTION_CANCEL:
                book.cancel(orderId);
                break;
            case ACTION_PRINT:
                book.print();
            default:
                cout<<"Unknown choice"<<endl;
                break;
        }                          
    }
    
    return 0;
}
/**
    bool match(Order o){
        book = null;
        if o.action == BUY
            head = sellOrders->head
        else
            head = buyOrders->head

        //check if we can fill
        filledQty=0, leaveQty=o.qty

        while priceMatch(head,o) && filledQty <= o.qty && (!head)
        {
            if (o.qty > head.qty)
                leaveQty -= filledQty
                filledQty += head.qty
                toDelete.append(head)
                head = head->next
            else
                filledQty += head.qty
                leaveQty -= filledQty
        }
        if leaveQty == 0
            for i in toDelete
                toDelete[i].clean()
                freedOrder.push(toDelete[i])
            o.clean()
            freedOrder.push(o)
            print("Trade")
            return FILLED
        else if o.orderType == IOC
            print("Cancelled")
            o.clean()
            freedOrder.push(o)
            return CANCELLED
        else 
            print("Unfilled")
            return UNFILLED
    }

    addToOrderBook(Order o){
        if o.action == BUY
            book = buyOrder->head
        else
            book = sellOrder->head

        added=False
        while (!added){
            if (o.price < book.price)
                order->next = book
                book=>order
                added = True
            }else if (o.price == book.price){
                prev=book
                tmp=book
                while (o.time < tmp.time && !tmp)
                    book=book->next
                
                order->next = book
                book=>order
                added = True
            }else{
                
            }
        }

    }
    addOrder(Order o){
        ret = match(o)
        if ret==UNFILLED
            //add to the Book
        else{
            if ret==CANCELLED
                print("CANCELLED")
            else
                print("FILLED")
        }
    }
}
**/