#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    

  
    
void settings_save();
void settings_load();
void settings_apply();

#define S_CHAR_LEN 50
#define N_ENTRY 10

class settings_entry{
public:
    settings_entry(){
        TR;
        count=0;
        val=0;
        name = new char[S_CHAR_LEN];
        value = new char *[N_ENTRY];
    }
    settings_entry(const char * n)
    {
        TR;
        count=0;
        name = new char[S_CHAR_LEN];
        value = new char *[N_ENTRY];
        strcpy(name,n);
    }
    void getName(char * dest){
        TR;
        printf("%p - %p\r\n",dest,name);
        strcpy(dest,name);
    }
    const char * getSelectedValueName(){
        return value[val];
    }
    settings_entry * setName(const char * n){
        TR;
        strcpy(name,n);
        TR;
        return this;
    }
    settings_entry * addValue(const char * v){
        value[count] = new char[S_CHAR_LEN];
        
        strcpy(value[count],v);
        count++;
        return this;
    }
    settings_entry * setValue(int v){
        TR;
        if(v>count)
            val=0;
        val=v;
        return this;
    }
    int getValue(){
        TR;
        return val;
    }
    int len(){
        TR;
        return count;
    }
private:
    char * name;
    int count;
    int val;
    char ** value; 
};

class settings{
public:
    settings(){
        count=0;
    }
    settings_entry * addEntry(const char * entry){
        TR;
        settings_entry * ret = &entries[count++];
        ret->setName(entry);
        TR;
        return ret;
    }
    
    settings_entry * getEntry(int at){
        if((at>0)&&(at<15))
            return &entries[at];
        else
            return NULL;
    }
    int len(){
        return count;
    }
private:
    int count;
    settings_entry entries[15];
    
};

extern settings * c_emu_settings;

#ifdef __cplusplus
}
#endif