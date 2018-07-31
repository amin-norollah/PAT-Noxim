#include "NoximThermal_IF.h"
#include <sys/stat.h>
//#include <direct.h>

extern double temp_budget[20][20][10];  // derek 12.10.16

void Thermal_IF::fpGenerator()
{
	double routerArea,	 memArea,	coreArea,	tileArea	;
	float routerWidth,	 memWidth,	coreWidth,	tileWidth	;
	float routerLength, memLength, coreLength, tileLength	;
	area.calculateArea();
    // Generate floorplan file of each layer for HotSpot (.flp)
	for(int k=0; k<mesh_dim_z; k++)
	{
		ofstream flp_file;
		ostringstream k_str;
		k_str<<k;
		string flpFileName = "mesh3d_"+k_str.str()+".flp";
		flp_file.open(flpFileName.c_str());
		if(!flp_file.is_open())
			cout<<"Cannot open "<<flpFileName.c_str()<<endl;
		else{
			for ( int j = 0 ; j < mesh_dim_y ; j++ )
			for ( int i = 0 ; i < mesh_dim_x ; i++ ){
				
				routerArea = ceil(area.getTotalRouterArea()*1e-03)*1e-09;
				//memArea = floor((AREA_DC + AREA_SQ + (7 * AREA_IFU) / 36)*1e-03)*1e-09;
				coreArea = floor(AREA_TOTAL *1e-03)	*1e-09;// -memArea;
				memArea  = floor(AREA_L2	*1e-03)	*1e-09;
				tileArea = coreArea + routerArea + memArea;


				routerWidth = ceil(sqrt(routerArea)*1e05+20)*1e-5;
				routerLength = routerWidth;

				memWidth = routerWidth;
				memLength = floor((memArea / memWidth)*1e05)* 1e-05;

				tileLength = routerLength + memLength;

				coreLength = tileLength;
				coreWidth = floor((coreArea / coreLength)*1e05)* 1e-05;

				tileWidth = coreWidth + routerWidth;

				//cout <<"tileLength "<< tileLength << "  routerLength " << routerLength << "   memLength " << memLength << endl;
				//cout << "tileLength " << tileLength << ",tileWidth " << tileWidth << ",coreLength " << coreLength << ",coreWidth " << coreWidth << endl;
				//cout << "routerLength " << routerLength << ",routerWidth "<< routerWidth << ",memLength " << memLength << ",memWidth " << memWidth << endl << endl;

				flp_file<<"router["<<i<<"]["<<j<<"]["<<k<<"]\t"<< routerWidth <<"\t"<< routerLength <<"\t"<< tileWidth*i<<"\t"<< tileLength*j<<"\n";
				flp_file<<"uP_mem["<<i<<"]["<<j<<"]["<<k<<"]\t"<< memWidth <<"\t"<< memLength <<"\t"<< tileWidth*i<<"\t"<< tileLength*j+ routerLength <<"\n";
				flp_file<<"uP_mac["<<i<<"]["<<j<<"]["<<k<<"]\t"<< coreWidth <<"\t"<< coreLength <<"\t"<< tileWidth*i+ routerWidth <<"\t"<< tileLength*j<<"\n";
			}

			/* with ring block version
			for (int j=0; j<mesh_dim_y; j++)
			for (int i=0; i<mesh_dim_x; i++){
				flp_file<<"router["<<i<<"]["<<j<<"]["<<k<<"]\t"<<ROUTER_WIDTH<<"\t"<<ROUTER_LENGTH<<"\t"<<TILE_WIDTH+TILE_WIDTH*i<<"\t"<<TILE_LENGTH+TILE_LENGTH*j<<"\n";
				flp_file<<"uP_mem["<<i<<"]["<<j<<"]["<<k<<"]\t"<<MEM_WIDTH<<"\t"<<MEM_LENGTH<<"\t"<<TILE_WIDTH+TILE_WIDTH*i<<"\t"<<TILE_LENGTH+TILE_LENGTH*j+ROUTER_LENGTH<<"\n";
				flp_file<<"uP_mac["<<i<<"]["<<j<<"]["<<k<<"]\t"<<FPMAC_WIDTH<<"\t"<<FPMAC_LENGTH<<"\t"<<TILE_WIDTH+TILE_WIDTH*i+ROUTER_WIDTH<<"\t"<<TILE_LENGTH+TILE_LENGTH*j<<"\n";
			}				
			flp_file<<"Null_block1["<<k<<"]\t"<<TILE_WIDTH*(mesh_dim_x+1)<<"\t"<<TILE_LENGTH<<"\t"<<0<<"\t"<<0<<"\n";
			flp_file<<"Null_block2["<<k<<"]\t"<<TILE_WIDTH<<"\t"<<TILE_LENGTH*(mesh_dim_y+1)<<"\t"<<0<<"\t"<<TILE_LENGTH<<"\n";	
			flp_file<<"Null_block3["<<k<<"]\t"<<TILE_WIDTH*(mesh_dim_x+1)<<"\t"<<TILE_LENGTH<<"\t"<<TILE_WIDTH<<"\t"<<TILE_LENGTH*(TGlobalParams::mesh_dim_y+1)<<"\n";
			flp_file<<"Null_block4["<<k<<"]\t"<<TILE_WIDTH<<"\t"<<TILE_LENGTH*(mesh_dim_y+1)<<"\t"<<TILE_WIDTH*(TGlobalParams::mesh_dim_x+1)<<"\t"<<0<<"\n";	
			*/
			flp_file.close();
		}
	}
	// Generate grid_layer_file for HotSpot (.lcf)
	ofstream lcf_file;
	lcf_file.open("mesh3d.lcf");
	if(!lcf_file.is_open())cout<<"Cannot open mesh3d.lcf"<<endl;
	else{
		for(int k = 0; k < mesh_dim_z ; k++ ){
			lcf_file<<"# Layer "<<k*2<<": Silicon\n";
			lcf_file<<k*2<<"\nY\nY\n"<<HEAT_CAP_SILICON<<"\n"<<RESISTIVITY_SILICON<<"\n"<<THICKNESS_SILICON<<"\nmesh3d_"<<k<<".flp\n\n";
			lcf_file<<"# Layer "<<k*2+1<<": thermal interface material (TIM)\n";
			lcf_file<<k*2+1<<"\nY\nN\n"<<HEAT_CAP_THERM_IF<<"\n"<<RESISTIVITY_THERM_IF<<"\n"<<THICKNESS_THERM_IF<<"\nmesh3d_"<<k<<".flp\n\n";
		}
		lcf_file.close();
	}
	// Generate initial temperature file for HotSpot (.init)

	ofstream init_temp_file;
	init_temp_file.open("mesh3d.init");
	if(!init_temp_file.is_open())
		cout<<"Cannot open mesh3d.init"<<endl;
	else
	{
  	for (int k=0; k<mesh_dim_z; k++){	  			
		  for (int j=0; j<mesh_dim_y; j++)
			  for (int i=0; i<mesh_dim_x; i++)		  
				{	
					if(k==0 || k==1)
					{
						init_temp_file<<"layer_"<<k*2<<"_router["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP_80<<endl;
						init_temp_file<<"layer_"<<k*2<<"_uP_mem["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP_80<<endl;
						init_temp_file<<"layer_"<<k*2<<"_uP_mac["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP_80<<endl;
					}
					else
					{
						init_temp_file<<"layer_"<<k*2<<"_router["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP<<endl;
						init_temp_file<<"layer_"<<k*2<<"_uP_mem["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP<<endl;
						init_temp_file<<"layer_"<<k*2<<"_uP_mac["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP<<endl;
					}
				}
			init_temp_file<<"layer_"<<k*2<<"_Null_block1["<<k<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"layer_"<<k*2<<"_Null_block2["<<k<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"layer_"<<k*2<<"_Null_block3["<<k<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"layer_"<<k*2<<"_Null_block4["<<k<<"]\t"<<INIT_TEMP<<endl;	
			for (int j=0; j<mesh_dim_y; j++)
			  for (int i=0; i<mesh_dim_x; i++)	
				{
					if(k==0 || k==1)
					{
						init_temp_file<<"layer_"<<k*2+1<<"_router["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP_80<<endl;
						init_temp_file<<"layer_"<<k*2+1<<"_uP_mem["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP_80<<endl;
						init_temp_file<<"layer_"<<k*2+1<<"_uP_mac["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP_80<<endl;
					}
					else
					{
						init_temp_file<<"layer_"<<k*2+1<<"_router["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP<<endl;
						init_temp_file<<"layer_"<<k*2+1<<"_uP_mem["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP<<endl;
						init_temp_file<<"layer_"<<k*2+1<<"_uP_mac["<<i<<"]["<<j<<"]["<<k<<"]\t"<<INIT_TEMP<<endl;
					}
				}
			init_temp_file<<"layer_"<<k*2+1<<"_Null_block1["<<k<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"layer_"<<k*2+1<<"_Null_block2["<<k<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"layer_"<<k*2+1<<"_Null_block3["<<k<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"layer_"<<k*2+1<<"_Null_block4["<<k<<"]\t"<<INIT_TEMP<<endl;	
			}
			for(int y=0; y < mesh_dim_y; y++) 
				for(int x=0; x < mesh_dim_x; x++)
				{
					init_temp_file<<"hsp_router["<<x<<"]["<<y<<"]["<< mesh_dim_z-1<<"]\t";//router
					init_temp_file<<INIT_TEMP<<"\n";
					init_temp_file<<"hsp_uP_mem["<<x<<"]["<<y<<"]["<< mesh_dim_z-1<<"]\t";//up_mem
					init_temp_file<<INIT_TEMP<<"\n";
					init_temp_file<<"hsp_uP_mac["<<x<<"]["<<y<<"]["<< mesh_dim_z-1<<"]\t";//up_mac
					init_temp_file<<INIT_TEMP<<"\n";
				}
			init_temp_file<<"hsp_Null_block1["<< mesh_dim_z-1<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"hsp_Null_block2["<< mesh_dim_z-1<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"hsp_Null_block3["<< mesh_dim_z-1<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"hsp_Null_block4["<< mesh_dim_z-1<<"]\t"<<INIT_TEMP<<endl;	
			for(int y=0; y < mesh_dim_y; y++) 
				for(int x=0; x < mesh_dim_x; x++)
				{
					init_temp_file<<"hsink_router["<<x<<"]["<<y<<"]["<< mesh_dim_z-1<<"]\t";//router
					init_temp_file<<INIT_TEMP<<"\n";
					init_temp_file<<"hsink_uP_mem["<<x<<"]["<<y<<"]["<< mesh_dim_z-1<<"]\t";//up_mem
					init_temp_file<<INIT_TEMP<<"\n";
					init_temp_file<<"hsink_uP_mac["<<x<<"]["<<y<<"]["<< mesh_dim_z-1<<"]\t";//up_mac
					init_temp_file<<INIT_TEMP<<"\n";
				}
			init_temp_file<<"hsink_Null_block1["<< mesh_dim_z-1<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"hsink_Null_block2["<< mesh_dim_z-1<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"hsink_Null_block3["<< mesh_dim_z-1<<"]\t"<<INIT_TEMP<<endl;	
			init_temp_file<<"hsink_Null_block4["<< mesh_dim_z-1<<"]\t"<<INIT_TEMP<<endl;	
		for(int i=0; i<12; i++)
			init_temp_file<<"inode_"<<i<<"\t"<<INIT_TEMP<<endl;
		init_temp_file.close();
	}
}


/*=====================================HotSpot initialization=====================*/
void Thermal_IF::hs_init()
{
	/* initialize flp, get adjacency matrix */
	hs_flp = read_flp("mesh3d_0.flp", FALSE); //Redundant file, overwrite by lcf file latter

	/* 
	 * configure thermal model parameters. default_thermal_config 
	 * returns a set of default parameters. only those configuration
	 * parameters (config.*) that need to be changed are set explicitly. 
	 */
	thermal_config_t hs_config = default_thermal_config();

	/*strcpy(config.init_file, init_file);
	strcpy(config.steady_file, steady_file);*/

	/* default_thermal_config selects block model as the default.
	 * in case grid model is needed, select it explicitly and
	 * set the grid model parameters (grid_rows, grid_cols, 
	 * grid_steady_file etc.) appropriately. for e.g., in the
	 * following commented line, we just choose the grid model 
	 * and let everything else to be the default. 
	 * NOTE: for modeling 3-D chips, it is essential to set
	 * the layer configuration file (grid_layer_file) parameter.
	 */
	strcpy(hs_config.model_type, GRID_MODEL_STR);
	strcpy(hs_config.grid_layer_file, "mesh3d.lcf");

	/* allocate and initialize the RC model	*/
	hs_model = alloc_RC_model(&hs_config, hs_flp);
	populate_R_model(hs_model, hs_flp);
	populate_C_model(hs_model, hs_flp);

	/* allocate the temp and power arrays	*/
	/* using hotspot_vector to internally allocate any extra nodes needed	*/
	hs_inst_temp = hotspot_vector(hs_model);
	hs_inst_power = hotspot_vector(hs_model);
	hs_steady_temp = hotspot_vector(hs_model);
	hs_overall_power = hotspot_vector(hs_model);
	
	/* set up initial instantaneous temperatures */
	//if (strcmp(model->config->init_file, NULLFILE)) {
	//	if (!model->config->dtm_used)	/* initial T = steady T for no DTM	*/
	//		read_temp(model, temp, model->config->init_file, FALSE);
	//	else	/* initial T = clipped steady T with DTM	*/
	//		read_temp(model, temp, model->config->init_file, TRUE);
	//}
	//else	/* no input file - use init_temp as the common temperature	*/
	
	set_temp(hs_model, hs_inst_temp, hs_model->config->init_temp);

	hs_first_call = TRUE;
}
/*===================================End of HS initialization=====================*/

/*===================================Compute Steady Temperature===================*/
void Thermal_IF::hs_steady( NoximTile *t[MAX_STATIC_DIM][MAX_STATIC_DIM][MAX_STATIC_DIM] )
{
	int i, j, base;
	int hs_simulation_time;
	
	if(!mkdir("results/STEADY",0777)) cout<<"Making new directory results/STEADY"<<endl;
	//if (!_mkdir("results/STEADY")) cout << "Making new directory results/STEADY" << endl;
	string file_name ("results/STEADY/SteadyTemp");
	file_name = MarkFileName( file_name );
	char * cstr;
	hs_simulation_time = int ( getCurrentCycleNum() / TEMP_REPORT_PERIOD ); //�������ơA���O�ɶ���cylce��
	/* find the average power dissipated in the elapsed time */
	for( i = 0 , base = 0 ; i < hs_model->grid->n_layers               ; i++ ){
		if(hs_model->grid->layers[i].has_power)
			for( j = 0    ; j < hs_model->grid->layers[i].flp->n_units ; j++ ){
				hs_overall_power[base+j] /= hs_simulation_time;
			}
		base += hs_model->grid->layers[i].flp->n_units;
	}
    /*Create the file name for each simulation.
	  Simulations with the same configuration will be recorded in the same file. 2009/12/8-Foster*/
	cstr = new char [file_name.size()+1];
	strcpy (cstr, file_name.c_str());	
	/* get steady state temperatures */
	steady_state_temp(hs_model, hs_overall_power, hs_steady_temp);
	//dump_overallpwr();
	dump_temp(hs_model, hs_steady_temp, cstr);
	dump_steady_temp_grid(hs_model->grid, "results/STEADY/grid_steady_temp");
	
	int o,n,m,r_nm,hs_idx;
	char router_name[30];
	results_log_router<<"#Steady State Temp.\n";
	int hs_base = 0;
	// for( hs_lyr=0, hs_base=0; hs_lyr < hs_model->grid->n_layers; hs_lyr++) {
	// results_log_router<<"hs_lyr:"<<hs_lyr<<endl;
	// if(hs_model->grid->layers[hs_lyr].has_power){
			
			for( o = 0 ; o < mesh_dim_z ; o++ ){
				results_log_router<<"XY"<<o<<" = [\n";
				for( n = mesh_dim_y - 1 ; n > -2  ; n-- ){
					for( m = 0 ; m < mesh_dim_x ; m++ ){
						r_nm = sprintf(router_name, "router[%d][%d][%d]", m, n, o);
						if( n != -1 ){
							hs_idx = get_blk_index(hs_model->grid->layers[2*o].flp, router_name);
							//Derek modified----
							/*if( (hs_steady_temp[hs_base + hs_idx ] - 273.15) > 98 )
								results_log_router<<"x"<<"\t";
							else
								results_log_router<<"."<<"\t";
							*/	
							 results_log_router << hs_steady_temp[hs_base + hs_idx ] - 273.15 <<"\t";
						}
						else
							results_log_router << "0\t";
					}
					results_log_router << "0\n";
				}
				results_log_router<<"]"<<endl;
				hs_base += hs_model->grid->layers[2*o].flp->n_units;
			}
			results_log_router<<"color_range = [25 65];"<<endl;
			results_log_router<<"figure(1)"<<endl;
			int temp = 1;
			for( o = 0 ; o < mesh_dim_z ; o++){
				results_log_router<<"subplot("<<mesh_dim_z<<",1,"<<temp<<"), pcolor(XY"<<o<<"), axis off, caxis( color_range ), colormap(jet)"<<endl;
				temp += 1;
			}
			results_log_router<<"set(gcf, 'PaperPosition', [1 1 7 30]);"<<endl;
			results_log_router<<"print(gcf,'-djpeg','-r0','"<<MarkFileName( string("SS") )<<"-"<<getCurrentCycleNum()<<".jpg')"<<endl;
		// }
		// hs_base += hs_model->grid->layers[hs_lyr].flp->n_units;
	// }
	results_log_router<<"\n";
}
/*============================End of Compute Steady Temperature===================*/

void Thermal_IF::logFile_init()
{
	//Transient temperature tracefile
	if (!mkdir("results", 0777)) cout << "Making new directory results" << endl;
	//if (!_mkdir("results")) cout << "Making new directory results" << endl;

	if(!mkdir("results/TEMP",0777)) cout<<"Making new directory results/TEMP"<<endl;
	//if (!_mkdir("results/TEMP")) cout << "Making new directory results/TEMP" << endl;
	//All element temperature log file
	string TEMP_filename;
	if( NoximGlobalParams::Log_all_Temp ){
		TEMP_filename = string("results/TEMP/TEMP");
		TEMP_filename = MarkFileName( TEMP_filename );
		results_log_temp.open( TEMP_filename.c_str(), ios::out );
		if(!results_log_temp.is_open())
			cout<<"Cannot open "<< TEMP_filename.c_str() <<endl;
		for(int z = 0; z < mesh_dim_z; z++)
		for(int y = 0; y < mesh_dim_y; y++)
		for(int x = 0; x < mesh_dim_x; x++){
			results_log_temp<<"router["<<x<<"]["<<y<<"]["<<z<<"]\t";
			results_log_temp<<"uP_mem["<<x<<"]["<<y<<"]["<<z<<"]\t";
			results_log_temp<<"uP_mac["<<x<<"]["<<y<<"]["<<z<<"]\t";
		}
		results_log_temp<<"\n";
	}
	//Router temperature log file
	TEMP_filename = string("results/TEMP/RouterTemp");
	TEMP_filename = MarkFileName( TEMP_filename );
	results_log_router.open( TEMP_filename.c_str(), ios::out );
	if(!results_log_router.is_open())
		cout<<"Cannot open "<< TEMP_filename.c_str() <<endl;

	//Core temperature log file
	TEMP_filename = string("results/TEMP/CoreTemp");
	TEMP_filename = MarkFileName(TEMP_filename);
	results_log_core.open(TEMP_filename.c_str(), ios::out);
	if (!results_log_core.is_open())
		cout << "Cannot open " << TEMP_filename.c_str() << endl;

	//Mem temperature log file
	TEMP_filename = string("results/TEMP/MemTemp");
	TEMP_filename = MarkFileName(TEMP_filename);
	results_log_mem.open(TEMP_filename.c_str(), ios::out);
	if (!results_log_mem.is_open())
		cout << "Cannot open " << TEMP_filename.c_str() << endl;
}

void Thermal_IF::hs_finish()
{
    delete_RC_model(hs_model        );
	free_flp       (hs_flp, FALSE   );
	free_dvector   (hs_inst_temp    );
	free_dvector   (hs_inst_power   );
	free_dvector   (hs_steady_temp  );
	free_dvector   (hs_overall_power);
}

void Thermal_IF::hs_temperature(vector<double>& instPwr)
{
   //convert powerTrace from Network Model to internal power represent format of HotSpot  
   char router_name[20], mem_name[20], mac_name[20]; //tile�ӼƤ���W�L�Q���  XD
   int r_nm, mem_nm, mac_nm;
   int m, n, o, hs_lyr, hs_base;
   int hs_idx;
   int router_chk, mem_chk, mac_chk;
   router_chk = 0;
   mem_chk = 0;
   mac_chk = 0;
   int idx = 0;
   
   for(hs_lyr=0, hs_base=0; hs_lyr < hs_model->grid->n_layers; hs_lyr++) { //���L�N�q�j����O�b�o
		if(hs_model->grid->layers[hs_lyr].has_power){
			for( o = 0 ; o < mesh_dim_z ; o ++ )
			for( n = 0 ; n < mesh_dim_y ; n ++ )
			for( m = 0 ; m < mesh_dim_x ; m ++ ){
				r_nm   = sprintf(router_name, "router[%d][%d][%d]", m, n, o);
				mem_nm = sprintf(mem_name   , "uP_mem[%d][%d][%d]", m, n, o);
				mac_nm = sprintf(mac_name   , "uP_mac[%d][%d][%d]", m, n, o);
				idx = 3*(m + n*mesh_dim_y + o*mesh_dim_y*mesh_dim_x);
				//router
				hs_idx = get_blk_index(hs_model->grid->layers[hs_lyr].flp, router_name);
				if(hs_idx != -1){
					hs_inst_power[hs_base+hs_idx] = instPwr[idx];
					hs_overall_power[hs_base+hs_idx] += hs_inst_power[hs_base+hs_idx];
				}
				//mem
				hs_idx = get_blk_index(hs_model->grid->layers[hs_lyr].flp, mem_name);
				if(hs_idx != -1){
					hs_inst_power[hs_base+hs_idx] = instPwr[idx+1];//uP_mem
					hs_overall_power[hs_base+hs_idx] += hs_inst_power[hs_base+hs_idx];
				}
				//mac
				hs_idx = get_blk_index(hs_model->grid->layers[hs_lyr].flp, mac_name);
				if(hs_idx != -1){
					hs_inst_power[hs_base+hs_idx] = instPwr[idx+2];//uP_mac
					hs_overall_power[hs_base+hs_idx] += hs_inst_power[hs_base+hs_idx];
				}
			}		
		}
		hs_base += hs_model->grid->layers[hs_lyr].flp->n_units;
	}
   if (hs_first_call)
		compute_temp(hs_model, hs_inst_power, hs_inst_temp, hs_model->config->sampling_intvl);
   else
		compute_temp(hs_model, hs_inst_power, NULL, hs_model->config->sampling_intvl);
	/* make sure to record the first call	*/
	hs_first_call = false;
}

void Thermal_IF::Tmp2TtraceFile(vector<double>& temperature)
{
	char router_name[30], mem_name[21], mac_name[21];
	int r_nm, mem_nm, mac_nm;
	int m, n, o, hs_lyr, hs_base;
	int hs_idx;
	int idx = 0;
	/***Temperature output from HotSpot model****/
	for(hs_lyr=0, hs_base=0; hs_lyr < hs_model->grid->n_layers; hs_lyr++) {
		if(hs_model->grid->layers[hs_lyr].has_power){
			for( o = 0 ; o < mesh_dim_z ; o++ )
			for( n = 0 ; n < mesh_dim_y ; n++ )
			for( m = 0 ; m < mesh_dim_x ; m++ ){
				r_nm   = sprintf(router_name, "router[%d][%d][%d]", m, n, o);
				mem_nm = sprintf(mem_name   , "uP_mem[%d][%d][%d]", m, n, o);
				mac_nm = sprintf(mac_name   , "uP_mac[%d][%d][%d]", m, n, o);
				idx = 3 * xyz2Id(m,n,o);
				//router
				hs_idx = get_blk_index(hs_model->grid->layers[hs_lyr].flp, router_name);
				if(hs_idx != -1){
					if( NoximGlobalParams::Log_all_Temp )
						results_log_temp  << hs_inst_temp[hs_base+hs_idx] - 273.15 <<"\t";
					temperature[idx  ] = hs_inst_temp[hs_base+hs_idx] - 273.15;   // TEMPERATURE INSERT
				}
				//mem
				hs_idx = get_blk_index(hs_model->grid->layers[hs_lyr].flp, mem_name);
				if(hs_idx != -1){
					if( NoximGlobalParams::Log_all_Temp )
						results_log_temp  << hs_inst_temp[hs_base+hs_idx] - 273.15 <<"\t";
					temperature[idx+1] = hs_inst_temp[hs_base+hs_idx] - 273.15;
				}
				//mac
				hs_idx = get_blk_index(hs_model->grid->layers[hs_lyr].flp, mac_name);
				if(hs_idx != -1){
					if( NoximGlobalParams::Log_all_Temp )
						results_log_temp  << hs_inst_temp[hs_base+hs_idx] - 273.15 <<"\t";
					temperature[idx+2] = hs_inst_temp[hs_base+hs_idx] - 273.15;							
				}
			}
		}
		hs_base += hs_model->grid->layers[hs_lyr].flp->n_units;
	}
	hs_lyr=0;
	hs_base=0;
	if(hs_model->grid->layers[hs_lyr].has_power){
		for( o = 0 ; o < mesh_dim_z ; o++ ){
			results_log_router<<"XY"<<o<<" = [\n";
			results_log_core << "XY" << o << " = [\n";
			results_log_mem << "XY" << o << " = [\n";

			for( n = mesh_dim_y - 1 ; n > -2  ; n-- ){
				for( m = 0 ; m < mesh_dim_x ; m++ ){
					if( n != -1 ){
						/*if( (hs_steady_temp[hs_base + hs_idx ] - 273.15) > 98 )
								results_log_router<<"x"<<"\t";
							else
								results_log_router<<"."<<"\t";
						*/		
						results_log_router << temperature[3 * xyz2Id(m, n, o)] << "\t";
						results_log_core   << temperature[3 * xyz2Id(m, n, o)+2] << "\t";
						results_log_mem    << temperature[3 * xyz2Id(m, n, o)+1] << "\t";
					}
					else {
						results_log_router << "0\t";
						results_log_core << "0\t";
						results_log_mem << "0\t";
					}
				}
				results_log_router << "0\n";
				results_log_core << "0\n";
				results_log_mem << "0\n";
			}
			results_log_router << "]" <<endl;
			results_log_core << "]" << endl;
			results_log_mem << "]" << endl;
		}
		results_log_router<<"color_range = [25 65];"<<endl;
		results_log_router<<"figure(1)"<<endl;
		results_log_core << "color_range = [25 65];" << endl;
		results_log_core << "figure(1)" << endl;
		results_log_mem << "color_range = [25 65];" << endl;
		results_log_mem << "figure(1)" << endl;
		int temp = 1;
		for( o = 0 ; o < mesh_dim_z ; o++){
			results_log_router << "subplot(" << mesh_dim_z << ",1," << temp << "), pcolor(XY" << o << "), axis off, caxis( color_range ), colormap(jet)" << endl;
			results_log_core << "subplot(" << mesh_dim_z << ",1," << temp << "), pcolor(XY" << o << "), axis off, caxis( color_range ), colormap(jet)" << endl;
			results_log_mem << "subplot(" << mesh_dim_z << ",1," << temp << "), pcolor(XY" << o << "), axis off, caxis( color_range ), colormap(jet)" << endl;
			temp += 1;
		}
		results_log_router<<"set(gcf, 'PaperPosition', [1 1 7 30]);"<<endl;
		results_log_router<<"print(gcf,'-djpeg','-r0','"<<MarkFileName( string("R") )<<"-"<<getCurrentCycleNum()<<".jpg')"<<endl;
		results_log_core << "set(gcf, 'PaperPosition', [1 1 7 30]);" << endl;
		results_log_core << "print(gcf,'-djpeg','-r0','" << MarkFileName(string("R")) << "-" << getCurrentCycleNum() << ".jpg')" << endl;
		results_log_mem << "set(gcf, 'PaperPosition', [1 1 7 30]);" << endl;
		results_log_mem << "print(gcf,'-djpeg','-r0','" << MarkFileName(string("R")) << "-" << getCurrentCycleNum() << ".jpg')" << endl;

	}
	results_log_router << "\n";
	results_log_core << "\n";
	results_log_mem << "\n";

	
}

