/* autoland.cp - host only: lands Mars Lander (Leisure Library program 14) on TI59Core with the module image
   build/leisure.bin (host-ref/mkmodule.cpp). Each turn it reads velocity.altitude off the display and burns what
   stops the craft at about 3 above ground, scaled by the burn effect of the last turn (the craft gets lighter).
     build/autoland <trigger>   e.g. 45: brake from the first turn -> touchdown at 1 with 65 of 630 fuel left
   (2026-10-26; the same burns flown on the guest gave the same log). */
#include "TI59Core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static TI59Core* m;
static void StepFor(u32 n){u32 i;for(i=0;i<n;i++)m->Step();}
static void RunToScan(void){u32 i;for(i=0;i<3000000UL;i++){m->Step();if(m->PC()>=kTI59ScanLoLo&&m->PC()<=kTI59ScanLoHi)return;}}
static void Tap(short c){m->PressMatrix(c);StepFor(1421);m->ReleaseMatrix(c);StepFor(1421);RunToScan();}
static void Disp(char*s){TI59Display d;short i,n=0;m->TakeDisplay(&d);StepFor(1421);m->TakeDisplay(&d);
 for(i=11;i>=0;i--){if(!d.litTicks[i])continue;switch(d.ctrl[i]){case 0:case 1:case 8:case 9:s[n++]=(char)('0'+d.digits[i]%10);break;case 5:case 6:s[n++]='-';break;}if(d.dpTicks[i])s[n++]='.';}s[n]=0;}
static const short kDig[10]={92,82,83,84,72,73,74,62,63,64};
static void Num(int b){char t[8];int i;sprintf(t,"%d",b);for(i=0;t[i];i++)Tap(kDig[t[i]-'0']);}
int main(int argc,char**argv){
 static u8 blob[kTI59BlobBytes],mod[5000]; char s[40],f[40]; double v,h,need; int b,turn,braking=0,lastb=0; double k=1.0,lastv=0; double trig=atof(argv[1]), extra=argc>2?atof(argv[2]):0;
 FILE*fp=fopen("build/ti59rom.bin","rb");fread(blob,1,sizeof blob,fp);fclose(fp);fp=fopen("build/leisure.bin","rb");fread(mod,1,5000,fp);fclose(fp);
 m=new TI59Core;m->LoadBlob(blob);m->LoadLibrary(mod,5000);m->PowerOn();m->SetPrinterConnected(0);RunToScan();
 Tap(21);Tap(31);Tap(82);Tap(72);Tap(15);Disp(s);
 for(turn=0;turn<40;turn++){
  char*p=strchr(s,'.'); if(!p||strlen(p)<2){int k;printf("end: [%s]",s);for(k=0;k<6;k++){StepFor(3000);Disp(s);printf(" [%s]",s);}printf("\n");break;}
  v=atof(s); {char a[5]="0000";int i;for(i=0;i<4&&p[1+i];i++)a[i]=p[1+i];h=atof(a);}
  need=(h>0&&v<-3)? (v*v-9.0)/(2*h):0; if(turn>0&&lastb>5)k=(v-lastv+13.0)/lastb; if(need>=trig)braking=1; b=(braking&&need>0)?(int)floor((need+13+extra)/k+0.5):0; if(b>75)b=75; if(b<0)b=0;
  printf("turn %2d  v=%5.0f h=%5.0f need=%5.1f k=%.2f  burn %d\n",turn,v,h,need,k,b); lastb=b; lastv=v;
  Num(b);Tap(11);Disp(s);
 }
 Tap(12);Disp(f);printf("fuel %s\n",f);return 0;}
