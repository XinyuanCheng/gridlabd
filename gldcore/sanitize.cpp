// sanitize.cpp
// Copyright (C) 2012 Battelle Memorial Institute
// Author DP Chassin
//

#include "gldcore.h"

// SET_MYCONTEXT(DMC_SANITIZE) // only used if IN_MYCONTEXT is present in this module

typedef struct s_safename {
	const char *name;
	const char *old;
	struct s_safename *next;
} SAFENAME;
static SAFENAME *safename_list = NULL;
static const char *sanitize_name(OBJECT *obj)
{
	SAFENAME *safe = (SAFENAME*)malloc(sizeof(SAFENAME));
	if ( !safe ) return NULL;
	safe->old = strdup(obj->name);
	char buffer[1024];
	sprintf(buffer,"%s%llX",global_sanitizeprefix.get_string(),(unsigned int64)safe);
	safe->name = object_set_name(obj,buffer);
	safe->next = safename_list;
	safename_list = safe;
	return safe->name;
}

/** sanitize

	Sanitizes a gridlabd model by clear names and position from object headers

    @returns 0 on success, -2 on error
 **/
int sanitize(void *main, int argc, const char *argv[])
{
	OBJECT *obj;
	FILE *fp;
	double delta_latitude, delta_longitude;

	// lat/lon change
	if ( strcmp(global_sanitizeoffset,"")==0 )
	{
		delta_latitude = random_uniform(NULL,-5,+5);
		delta_longitude = random_uniform(NULL,-180,+180);
	}
	else if ( global_sanitizeoffset=="destroy" )
		delta_latitude = delta_longitude = QNAN;
	else if ( sscanf(global_sanitizeoffset.get_string(),"%lf%*[,/]%lf",&delta_latitude,&delta_longitude)!=2 )
	{
		output_error("sanitize_offset lat/lon '%s' is not valid", global_sanitizeoffset.get_string());
		return -2;
	}

	// sanitize object names
	for ( obj=object_get_first() ; obj!=NULL ; obj=object_get_next(obj) )
	{
		if ( obj->name!=NULL && (global_sanitizeoptions&SO_NAMES)==SO_NAMES )
			sanitize_name(obj);
		if ( isfinite(obj->latitude) && (global_sanitizeoptions&SO_GEOCOORDS)==SO_GEOCOORDS )
		{
			obj->latitude += delta_latitude;
			if ( obj->latitude<-90 ) obj->latitude = -90;
			if ( obj->latitude>+90 ) obj->latitude = +90;
		}
		if ( isfinite(obj->longitude) && (global_sanitizeoptions&SO_GEOCOORDS)==SO_GEOCOORDS )
			obj->longitude = fmod(obj->longitude+delta_longitude,360);
	}

	// dump object name index
	if ( global_sanitizeindex == ".xml" )
	{
		global_sanitizeindex = global_modelname;
		global_sanitizeindex.copy_from("-index.xml",global_sanitizeindex.findrev(".xml"));
	}
	else if ( global_sanitizeindex == ".txt" )
	{
		global_sanitizeindex = global_modelname;
		global_sanitizeindex.copy_from("-index.txt",global_sanitizeindex.findrev(".txt"));
	}
	else if ( global_sanitizeindex == ".json" )
	{
		global_sanitizeindex = global_modelname;
		global_sanitizeindex.copy_from("-index.json",global_sanitizeindex.findrev(".json"));
	}
	else if ( global_sanitizeindex[0] == '.' )
	{
		output_error("sanitization index file spec '%s' is not recognized", global_sanitizeindex.get_string());
		return -2;
	}
	if ( global_sanitizeindex != "" )
	{
		fp = fopen(global_sanitizeindex,"w");
		if ( fp )
		{
			SAFENAME *item;
			if ( varchar(global_sanitizeindex.substr(-4)) == ".xml" )
			{
				fprintf(fp,"<data>\n");
				fprintf(fp,"\t<modelname>%s</modelname>\n",global_modelname);
				fprintf(fp,"\t<geographic_offsets>\n");
				fprintf(fp,"\t\t<latitude>%.6f</latitude>\n",delta_latitude);
				fprintf(fp,"\t\t<longitude>%.6f</longitude>\n",delta_longitude);
				fprintf(fp,"\t</geographic_offsets>\n");
				fprintf(fp,"\t<safename_list>\n");
				for ( item=safename_list ; item!=NULL ; item=item->next )
					fprintf(fp,"\t\t<name>\n\t\t\t<safe>%s</safe>\n\t\t\t<unsafe>%s</unsafe>\n\t\t</name>\n", item->name, item->old);
				fprintf(fp,"\t</safename_list>\n");
				fprintf(fp,"</data>\n");
			}
			else if ( varchar(global_sanitizeindex.substr(-4)) == ".json" )
			{
				fprintf(fp,"{\n");
				fprintf(fp,"  \"modelname\" : \"%s\",\n",global_modelname);
				fprintf(fp,"  \"geographic_offsets\" : {\n");
				fprintf(fp,"    \"latitude\" : %.6f,\n",delta_latitude);
				fprintf(fp,"    \"longitude\" : %.6f\n",delta_longitude);
				fprintf(fp,"  }\n");
				fprintf(fp,"  \"safename_map\" : {\n");
				for ( item=safename_list ; item!=NULL ; item=item->next )
					fprintf(fp,"    \"%s\" : \"%s\"%s\n", item->old, item->name, item->next != NULL ? "," : "");
				fprintf(fp,"  }\n");
				fprintf(fp,"}\n");
			}
			else
			{
				fprintf(fp,"modelname\t= %s\n", global_modelname);
				fprintf(fp,"\n[POSITIONS]\n");
				fprintf(fp,"latitude\t= %.6f\n",delta_latitude);
				fprintf(fp,"longitude\t= %.6f\n",delta_longitude);
				fprintf(fp,"\n[NAMES]\n");
				for ( item=safename_list ; item!=NULL ; item=item->next )
					fprintf(fp,"%s\t= %s\n", item->name, item->old);
			}
			fclose(fp);
		}
	}

	return 0;
}
