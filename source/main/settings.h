#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    

  
    
void settings_save();
void settings_load();
void settings_apply();


class settings_entry{
public:
    settings_entry(){
        TR;
        count=0;
        val=0;
    }
    settings_entry(const char * n)
    {
        TR;
        count=0;
        strcpy(name,n);
    }
    void getName(char * dest){
        TR;
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
    char name[50];
    int count;
    int val;
    char value[10][50]; 
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

extern settings c_emu_settings;

#ifdef __cplusplus
}
#endif