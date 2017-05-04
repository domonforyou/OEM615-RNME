#include "Gnss_nav.h"
#include "gpsNav.h"
#include "util.h"

// inbuf: 85 bit (0,1); outbuf: 200bit ('0','1')
static int gps_simple_encode(char *inbuf,char *outbuf){

	for(int i=0;i<30;i++)
		outbuf[i] = inbuf[i]+48;

	return 0;
}

void parse_gpslog_header(char *header, GPS_INFO &info){
	int cnt=0;
	char *p1;

	if((p1 = strtok(header, ","))) {
		cnt++;
	}
	while((p1 = strtok(NULL, ","))) {
		if(cnt==4 && strncmp(p1,"FINESTEERING",strlen("FINESTEERING"))!=0){
			cout << "ERROR:: Parse GPS Log Header Failed, SATTIME Wrong !!!" << endl;
			break;
		}
		else if(cnt==5)
			sscanf(p1,"%d",&info.week);
		else if(cnt==6)
			sscanf(p1,"%lf",&info.tow);
		cnt++;
	}
	if(cnt!=GPS_HEADER_SLICE)
		cout << "ERROR:: Parse GPS Log Header Failed, Slice Wrong !!!" << endl;
}

int parse_gpslog_body(char *body, GPS_INFO &info){
	int cnt=0,sat_id=0;
	char *p1;

	if((p1 = strtok(body, ","))) {
		sscanf(p1,"%d",&sat_id);
		cnt++;
	}
	while((p1 = strtok(NULL, ","))) {
		if(cnt==1)
			strcpy(info.nav, p1);
		cnt++;
	}
	if(!strtok(info.nav, "*")){
		cout << "ERROR:: Parse GPS Log Body Failed, Nav Wrong !!!" << endl;
		return -1;
	}
	if(cnt!=GPS_BODY_SLICE){
		cout << "ERROR:: Parse GPS Log Body Failed, Slice Wrong !!!" << endl;
		return -1;
	}
	return sat_id;
}
/**********************************
|---------------------------------
	|-----------------------------
		|-------------------------
			|-------- |-----------
|--------------- |----------------

		|--->   final data   <---|
return:line record           line_end
**********************************/
int preparse_oem615_gpslog(const char *file, set<int> &final_sat_set, int &line_end, int min_time_len, int min_sat_num){

	FILE *fp;
	int sat_id,sat_num=0;
	int i,week,ret_line=0,cnt=0;
	int is_sat_exist[MAX_SAT_ID]={0};
	int is_sat_healthy[MAX_SAT_ID]={0};
	int diff_tow_cnt=0;
	double tow=0;
	double first_tow[MAX_SAT_ID]={0};
	double last_tow[MAX_SAT_ID]={0};
	int duration[MAX_SAT_ID]={0};
	int line_record[MAX_SAT_ID]={0};
	bool is_init_ok = false;
	char *s,*p1,*id;
	GPS_INFO info={0};
	//every record time delta *10, avoiding double not precise 
	int delta=6;
	char line[MAX_BUF];
	set<int> sat_set;

	if (!(fp=fopen(file,"r"))) {
		fprintf(stderr,"gps file open error : %s\n",file);
		return -1;
	}
	while (!feof(fp) && fgets(line, MAX_BUF, fp)){
		//skip some lines
		if(cnt++ < SKIP_LINES)
			continue;
		//find bds sign
		if(strncmp(line,GPS_HEADER,strlen(GPS_HEADER))!=0)
			continue;

		s = line;
		//change the line end to string end
		if (s[strlen(s) - 1] == '\n')
			s[strlen(s) - 1] = '\0';
		//split header and body
		if ((p1 = strchr(s, ';'))) {
			p1[0] = '\0';
		}
		//store xxx in info
		parse_gpslog_header(s, info);

		if(p1){
			p1++;
			sat_id = parse_gpslog_body(p1, info);
		}
		//init progress
		if(!is_init_ok){
			if(info.tow!=tow){
				tow=info.tow;
				diff_tow_cnt++;
				diff_tow_cnt > 4 ? is_init_ok=true : is_init_ok=false;
			}
			else{
				continue;
			}
		}
		//main process
		if(is_init_ok){
			if(!is_sat_exist[sat_id]){
				is_sat_exist[sat_id]=1;
				duration[sat_id]=0;
				first_tow[sat_id]=info.tow;
				last_tow[sat_id]=info.tow;
				line_record[sat_id]=cnt;
				sat_set.insert(sat_id);
				sat_num++;
				//infos.insert(make_pair(sat_id, info));
			}
			else{
				int delta_mul_10 = (int)((info.tow-last_tow[sat_id])*10+0.5);
				if(delta_mul_10 == delta)
					duration[sat_id] = info.tow - first_tow[sat_id];
				else{
					is_sat_exist[sat_id]=0;
					sat_set.erase(sat_id);
					sat_num--;
				}
				last_tow[sat_id]=info.tow;
			}
			if(sat_num >= min_sat_num && duration[sat_id] > min_time_len){
				int find_cnt=0;
				for(set<int>::iterator set_it = sat_set.begin(); set_it != sat_set.end(); set_it++){
					if(duration[*set_it]>min_time_len)
						find_cnt++;
				}
				if(find_cnt>=min_sat_num){
					for(set<int>::iterator set_it = sat_set.begin(); set_it != sat_set.end(); set_it++){
						if(duration[*set_it]>min_time_len){
							final_sat_set.insert(*set_it);
							ret_line = max(line_record[*set_it], ret_line);
						}
					}
					cout << "INFO:: Find OK GPS NAV DATA In The Log File !!!" << endl;
					fclose(fp);
					line_end = cnt;
					return ret_line;
				}
			}
		}
	}
	if(cnt < 500){
		fprintf(stderr,"file: %s is too short \n",file);
		fclose(fp);
		return -2;
	}
	fclose(fp);
	return -3;
}

int parse_oem615_gpslog(const char *file, int min_time_len, int min_sat_num){

	FILE *fp;
	FILE *fps[MAX_SAT_ID]={NULL};
	char *s,*p1;
	int is_sat_exist[MAX_SAT_ID]={0};
	int cnt = 0,sat_id,line_begin,line_end;
	char line[MAX_BUF];
	char data[64];
	char encode_data[64];
	GPS_INFO info={0};
	set<int> sat_set;

	line_begin = preparse_oem615_gpslog(file, sat_set, line_end, min_time_len, min_sat_num);
	if(line_begin<0){
		cout << "ERROR:: Preparse gps Log Failed !!!" << endl;
		return -1;
	}
	//open many sat nav txt file to store nav data
	for(set<int>::iterator set_it = sat_set.begin(); set_it != sat_set.end(); set_it++){
		is_sat_exist[*set_it] = 1;
		stringstream f;
		f << *set_it << ".txt";
		if (!(fps[*set_it]=fopen(f.str().c_str(),"w"))) {
			fprintf(stderr,"result file open error : %s\n",f.str().c_str());
			return -1;
		}
		f.str("");
		f << *set_it << "#";
		fwrite(f.str().c_str(), sizeof(char), f.str().size(), fps[*set_it]);
	}
	if (!(fp=fopen(file,"r"))) {
		fprintf(stderr,"e1b file open error : %s\n",file);
		return -1;
	}
	while (!feof(fp) && fgets(line, MAX_BUF, fp)){
		//skip some lines
		if(cnt++ < line_begin)
			continue;
		//end in line_end
		if(cnt > line_end)
			break;
		//find bds sign
		if(strncmp(line,GPS_HEADER,strlen(GPS_HEADER))!=0)
			continue;

		s = line;
		//change the line end to string end
		if (s[strlen(s) - 1] == '\n')
			s[strlen(s) - 1] = '\0';
		//split header and body
		if ((p1 = strchr(s, ';'))) {
			p1[0] = '\0';
		}
		//store xxx in info
		parse_gpslog_header(s, info);
		if(p1){
			p1++;
			sat_id = parse_gpslog_body(p1, info);
			if(is_sat_exist[sat_id]){
				hex_str2bin(info.nav,strlen(info.nav),data);
				//ignore first 2bit in 32bit
				gps_simple_encode(data+2,encode_data);
				fwrite(encode_data, sizeof(char), 30, fps[sat_id]);
			}
		}
		
	}
	for(set<int>::iterator set_it = sat_set.begin(); set_it != sat_set.end(); set_it++)
		fclose(fps[*set_it]);

	if(cnt < 500){
		fprintf(stderr,"file: %s is too short \n",file);
		return -1;
	}
	if(union_all_sat_nav(sat_set, "GPSNAV.txt")!=0){
		fprintf(stderr,"union_all_sat_nav failed \n");
		return -1;
	}
	return 0;
}