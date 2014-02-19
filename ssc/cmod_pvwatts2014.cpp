#include "core.h"

#include "lib_weatherfile.h"
#include "lib_irradproc.h"
#include "lib_pvwatts.h"
#include "lib_pvshade.h"

#ifndef DTOR
#define DTOR 0.0174532925
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif
#define sind(x) sin( (M_PI/180.0)*(x) )
#define cosd(x) cos( (M_PI/180.0)*(x) )
#define tand(x) tan( (M_PI/180.0)*(x) )
#define asind(x) (180/M_PI*asin(x))

static var_info _cm_vtab_pvwatts2014[] = {
/*   VARTYPE           DATATYPE         NAME                         LABEL                                               UNITS     META                      GROUP          REQUIRED_IF                 CONSTRAINTS                      UI_HINTS*/
	{ SSC_INPUT,        SSC_STRING,      "solar_resource_file",                      "local weather file path",                     "",       "",                        "Weather",      "*",                       "LOCAL_FILE",      "" },

	{ SSC_INPUT,        SSC_NUMBER,      "albedo",                         "Albedo (ground reflectance)",                 "frac",   "",                        "PVWatts",      "?",                       "",                                         "" },
	{ SSC_INPUT,        SSC_NUMBER,      "system_size",                    "Nameplate capacity",                          "kW",     "",                        "PVWatts",      "*",                       "MIN=0.05,MAX=500000",                      "" },
	{ SSC_INPUT,        SSC_NUMBER,      "dc_ac_ratio",                    "DC to AC ratio",                              "ratio",  "",                        "PVWatts",      "?=1.1",                   "POSITIVE",                                 "" },
	{ SSC_INPUT,        SSC_NUMBER,      "derate",                         "System derate value",                         "frac",   "",                        "PVWatts",      "*",                       "MIN=0,MAX=1",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "track_mode",                     "Tracking mode",                               "0/1/2/3","Fixed,1Axis,2Axis,AziAxis","PVWatts",      "*",                       "MIN=0,MAX=3,INTEGER",                      "" }, 
	{ SSC_INPUT,        SSC_NUMBER,      "azimuth",                        "Azimuth angle",                               "deg",    "E=90,S=180,W=270",        "PVWatts",      "*",                       "MIN=0,MAX=360",                            "" },
	{ SSC_INPUT,        SSC_NUMBER,      "tilt",                           "Tilt angle",                                  "deg",    "H=0,V=90",                "PVWatts",      "naof:tilt_eq_lat",        "MIN=0,MAX=90",                             "" },

	/* shading inputs */
	{ SSC_INPUT,        SSC_ARRAY,       "shading_hourly",                 "Hourly beam shading factors",                 "",       "",                        "PVWatts",      "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_MATRIX,      "shading_mxh",                    "Month x Hour beam shading factors",           "",       "",                        "PVWatts",      "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_MATRIX,      "shading_azal",                   "Azimuth x altitude beam shading factors",     "",       "",                        "PVWatts",      "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "shading_diff",                   "Diffuse shading factor",                      "",       "",                        "PVWatts",      "?",                        "",                              "" },
		
	/* advanced parameters */
	{ SSC_INPUT,        SSC_NUMBER,      "rotlim",                         "Tracker rotation limit (+/- 1 axis)",         "deg",    "",                        "PVWatts",      "?=45.0",                  "MIN=1,MAX=90",                             "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inoct",                          "Nominal operating cell temperature",          "C",      "",                        "PVWatts",      "?=45.0",                  "POSITIVE",                                 "" },
	{ SSC_INPUT,        SSC_NUMBER,      "gamma",                          "Max power temperature coefficient",           "%/C",    "",                        "PVWatts",      "?=-0.5",                  "",                                         "" },
	{ SSC_INPUT,        SSC_NUMBER,      "fd",                             "Diffuse fraction",                            "0..1",   "",                        "PVWatts",      "?=1.0",                   "MIN=0,MAX=1",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "w_stow",                         "Wind stow speed",                             "m/s",    "",                        "PVWatts",      "?=0",                     "MIN=0",                                    "" },
	{ SSC_INPUT,        SSC_NUMBER,      "concen",                         "Concentration ratio",                         "",       "",                        "PVWatts",      "?=1",                     "MIN=1",                                    "" },
	{ SSC_INPUT,        SSC_NUMBER,      "fhconv",                         "Convective heat transfer factor",             "",       "",                        "PVWatts",      "?=1",                     "MIN=0.1",                                  "" },
	{ SSC_INPUT,        SSC_NUMBER,      "shade_mode_1x",                  "Tracker self-shading mode",                   "0/1/2",  "0=shading,1=backtrack,2=none","PVWatts",  "?=2",                     "INTEGER,MIN=0,MAX=2",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "gcr",                            "Ground coverage ratio",                       "0..1",   "",                            "PVWatts",  "?=0.4",                   "MIN=0,MAX=3",               "" },
	{ SSC_INPUT,        SSC_NUMBER,      "ar_glass",                       "Enable anti-reflective glass coating",        "0/1",    "",                        "PVWatts",      "?=0",                     "BOOLEAN",                   "" },
	
	
	/* outputs */

	{ SSC_OUTPUT,       SSC_ARRAY,       "gh",                             "Global horizontal irradiance",                "W/m2",   "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "dn",                             "Beam irradiance",                             "W/m2",   "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "df",                             "Diffuse irradiance",                          "W/m2",   "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "tamb",                           "Ambient temperature",                         "C",      "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "tdew",                           "Dew point temperature",                       "C",      "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "wspd",                           "Wind speed",                                  "m/s",    "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },

	{ SSC_OUTPUT,       SSC_ARRAY,       "poa",                            "Plane of array irradiance",                   "W/m2",   "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "tpoa",                           "Transmitted plane of array irradiance",       "W/m2",   "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "tcell",                          "Module temperature",                          "C",      "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },	
	{ SSC_OUTPUT,       SSC_ARRAY,       "dc",                             "DC array output",                             "Wdc",    "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "ac",                             "AC system output",                            "Wac",    "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "shad_beam_factor",               "Shading factor for beam radiation",           "",       "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "sunup",                          "Sun up over horizon",                         "0/1",    "",                        "PVWatts",      "*",                       "LENGTH=8760",                          "" },

	{ SSC_OUTPUT,       SSC_ARRAY,       "poa_monthly",                    "Plane of array irradiance",                   "kWh/m2",   "",                        "PVWatts",      "*",                       "LENGTH=12",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "solrad_monthly",                 "Daily average solar irradiance",              "kWh/m2/day","",                        "PVWatts",      "*",                       "LENGTH=12",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "dc_monthly",                     "DC array output",                             "kWhdc",    "",                        "PVWatts",      "*",                       "LENGTH=12",                          "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "ac_monthly",                     "AC system output",                            "kWhac",    "",                        "PVWatts",      "*",                       "LENGTH=12",                          "" },

	{ SSC_OUTPUT,       SSC_NUMBER,      "solrad_annual",                  "Daily average solar irradiance",              "kWh/m2/day",    "",                        "PVWatts",      "*",                       "",                          "" },
	{ SSC_OUTPUT,       SSC_NUMBER,      "ac_annual",                      "Annual AC system output",                     "kWhac",    "",                        "PVWatts",      "*",                       "",                          "" },

	{ SSC_OUTPUT,       SSC_STRING,      "location",                      "Location ID",                                  "",    "",                        "PVWatts",      "*",                       "",                          "" },
	{ SSC_OUTPUT,       SSC_STRING,      "city",                          "City",                                         "",    "",                        "PVWatts",      "*",                       "",                          "" },
	{ SSC_OUTPUT,       SSC_STRING,      "state",                         "State",                                        "",    "",                        "PVWatts",      "*",                       "",                          "" },
	{ SSC_OUTPUT,       SSC_NUMBER,      "lat",                           "Latitude",                                     "deg", "",                        "PVWatts",      "*",                       "",                          "" },
	{ SSC_OUTPUT,       SSC_NUMBER,      "lon",                           "Longitude",                                    "deg", "",                        "PVWatts",      "*",                       "",                          "" },
	{ SSC_OUTPUT,       SSC_NUMBER,      "tz",                            "Time zone",                                    "hr",  "",                        "PVWatts",      "*",                       "",                          "" },
	{ SSC_OUTPUT,       SSC_NUMBER,      "elev",                          "Site elevation",                               "m",   "",                        "PVWatts",      "*",                       "",                          "" },


	var_info_invalid };

class cm_pvwatts2014 : public compute_module
{
public:
	
	cm_pvwatts2014()
	{
		add_var_info( _cm_vtab_pvwatts2014 );
	}

	double transmittance( double theta1, double n_material, double n_incoming, double k, double l, double *theta2_ret = 0 )
	{
		double theta2 = asind( n_incoming / n_material * sind(theta1 ) ); // % snell's law, assuming n_air = 1.0
		if ( theta2_ret != 0 ) *theta2_ret = theta2;

		// fresnel's equation for non-reflected unpolarized radiation as an average of perpendicular and parallel components
		double tr0 = 1 - 0.5 *
			( pow( sind(theta2-theta1), 2 )/pow( sind(theta2+theta1), 2)
			+ pow( tand(theta2-theta1), 2 )/pow( tand(theta2+theta1), 2 ) );

		return tr0; //* exp( -k * l / cosd(theta2) );
	}

	double iam( double theta, bool ar_glass )
	{
		double normal = iam_nonorm( 1, ar_glass );
		double actual = iam_nonorm( theta, ar_glass );
		return actual/normal;	
	}

	double iam_nonorm( double theta, bool ar_glass )
	{
		double n_air = 1.0;

		double n_g = 1.526;
		double k_g = 4;
		double l_g = 0.002;

		double n_arc = 1.3;
		double k_arc = 4;
		double l_arc = l_g*0.01;  // assume 1/100th thickness of glass for AR coating

		if ( theta < 1 ) theta = 1;
		if ( theta > 89 ) theta = 89;

		if ( ar_glass )
		{
			double theta2 = 1;
			double tau_coating = transmittance( theta, n_arc, n_air, k_arc, l_arc, &theta2 );
			double tau_glass = transmittance( theta2, n_g, n_arc, k_g, l_g );
			return tau_coating*tau_glass;
		}
		else
		{
			return transmittance(theta, n_g, n_air, k_g, l_g );
		}
	}

	void exec( ) throw( general_error )
	{
		const char *file = as_string("solar_resource_file");

		weatherfile wf( file );
		if (!wf.ok()) throw exec_error("pvwatts2014", "failed to read local weather file: " + std::string(file));
					
		double derate = as_double("derate");
		if ( derate < 0.0 || derate > 1.0 ) // Use if default ac to dc derate factor out of range
			derate = 0.86;
        
		int track_mode = as_integer("track_mode"); // 0, 1, 2, 3
		
		double azimuth = as_double("azimuth");
		double tilt = fabs(wf.lat);
		
		double t_ref = 25;
		double i_ref = 1000;
		
		double inoct = as_double("inoct") + 273.15; // PVWATTS_INOCT;        /* Installed normal operating cell temperature (deg K) */
		double gamma = as_double("gamma") / 100.0; // PVWATTS_PWRDGR;              /* Power degradation due to temperature (decimal fraction), si approx -0.004 */
	
		double height = PVWATTS_HEIGHT;                 /* Average array height (meters) */
		double rlim = as_double("rotlim");             /* +/- rotation in degrees permitted by physical constraint of tracker */
		double fd = as_double("fd"); // diffuse fraction
		double wind_stow = as_double("w_stow"); // maximum wind speed before stowing.  stowing causes all output to be lost

		double concen = 1.0;
		if ( is_assigned("concen") ) concen = as_double("concen"); // concentration ratio.  used to increase incident irradiance on cells for thermal calculaton		
		
		double fhconv = 1.0;
		if ( is_assigned("fhconv") ) fhconv = as_double("fhconv"); // convective heat transfer coefficient factor.  used to approximate effect of a heatsink for lcpv
		
		int shade_mode_1x = 2; // no self shading on 1 axis tracker
		if ( is_assigned("shade_mode_1x") ) shade_mode_1x = as_integer("shade_mode_1x");
		
		double gcr = 0.3;
		if ( is_assigned("gcr") ) gcr = as_double("gcr");

		bool use_ar_glass = false;
		if ( is_assigned("ar_glass") ) use_ar_glass = as_boolean("ar_glass");

		
		double dc_nameplate = as_double("system_size")*1000;
		double dc_ac_ratio = as_double("dc_ac_ratio");

		// check system size
		if ( dc_nameplate < 0.1 ) dc_nameplate = 0.1;

		double ac_nameplate = dc_nameplate / dc_ac_ratio;      // rated output of inverter in a.c. watts; 6/29/2005

		if( track_mode < 0 || track_mode > 3 )
			track_mode = 0;
		if( tilt < 0 || tilt > 90 )
			tilt = wf.lat;
		if( azimuth < 0 || azimuth > 360 )
			azimuth = 180.0;

		/* allocate hourly output arrays */
		
		ssc_number_t *p_gh = allocate("gh", 8760);
		ssc_number_t *p_dn = allocate("dn", 8760);
		ssc_number_t *p_df = allocate("df", 8760);
		ssc_number_t *p_tamb = allocate("tamb", 8760);
		ssc_number_t *p_tdew = allocate("tdew", 8760);
		ssc_number_t *p_wspd = allocate("wspd", 8760);

		ssc_number_t *p_dc = allocate("dc", 8760);
		ssc_number_t *p_ac = allocate("ac", 8760);
		ssc_number_t *p_tcell = allocate("tcell", 8760);
		ssc_number_t *p_poa = allocate("poa", 8760);
		ssc_number_t *p_tpoa = allocate("tpoa", 8760);

		ssc_number_t *p_shad_beam_factor = allocate("shad_beam_factor", 8760);
		ssc_number_t *p_sunup = allocate("sunup", 8760);
	
						
		
//		initialize to no shading
		for ( size_t j=0;j<8760;j++)
			p_shad_beam_factor[j] = 1.0;

		if ( is_assigned("shading_hourly" ) )
		{
			size_t len = 0;
			ssc_number_t *vals = as_array( "shading_hourly", &len );
			if ( len == 8760 )
			{
				for ( size_t j=0;j<8760;j++)
					p_shad_beam_factor[j] = vals[j];
			}
			else
				throw exec_error("pvwattsv1", "hourly shading beam factors must have 8760 values");
		}


		if ( is_assigned( "shading_mxh" ) )
		{
			size_t nrows, ncols;
			ssc_number_t *mat = as_matrix( "shading_mxh", &nrows, &ncols );
			if ( nrows != 12 || ncols != 24 )
				throw exec_error("pvwattsv1", "month x hour shading factors must have 12 rows and 24 columns");

			int c=0;
			for (int m=0;m<12;m++)
				for (int d=0;d<util::nday[m];d++)
					for (int h=0;h<24;h++)
						p_shad_beam_factor[c++] *= mat[ m*ncols + h ];
		}

		bool enable_azalt_beam_shading = false;
		util::matrix_t<double> azaltvals;
		if ( is_assigned( "shading_azal" ) )
		{
			size_t nrows, ncols;
			ssc_number_t *mat = as_matrix( "shading_azal", &nrows, &ncols );
			if ( nrows < 3 || ncols < 3 )
				throw exec_error("pvwattsv1", "azimuth x altitude shading factors must have at least 3 rows and 3 columns");

			azaltvals.resize_fill( nrows, ncols, 1.0 );
			for ( size_t r=0;r<nrows;r++ )
				for ( size_t c=0;c<ncols;c++ )
					azaltvals.at(r,c) = mat[r*ncols+c];

			enable_azalt_beam_shading = true;
		}

		double shad_skydiff_factor = 1.0;
		if ( is_assigned( "shading_diff" ) )
			shad_skydiff_factor = as_double( "shading_diff" );

		pvwatts_celltemp tccalc( inoct, height, 1.0 );
		
		double fixed_albedo = 0.2;
		bool has_albedo = is_assigned( "albedo" );
		if ( has_albedo )
			fixed_albedo = as_double( "albedo" );

	
		int i=0;
		while( i < 8760 )
		{
			if (!wf.read())
				throw exec_error("pvwatts2014", "could not read data line " + util::to_string(i+1) + " of 8760 in weather file");

			irrad irr;
			irr.set_time( wf.year, wf.month, wf.day, wf.hour, wf.minute, wf.step / 3600.0 );
			irr.set_location( wf.lat, wf.lon, wf.tz );
				
			double alb = 0.2; // do not increase albedo if snow exists in TMY2
			
			if ( has_albedo && fixed_albedo >= 0 && fixed_albedo <= 1.0 )
			{
				alb = fixed_albedo;
			}
			else if ( wf.type() == weatherfile::TMY3 
				|| wf.type() == weatherfile::WFCSV )
			{
				if ( wf.albedo >= 0 && wf.albedo < 1 )
					alb = wf.albedo;
			}

			irr.set_sky_model( 2, alb );
			irr.set_beam_diffuse( wf.dn, wf.df );
			irr.set_surface( track_mode, tilt, azimuth, rlim, 
				shade_mode_1x == 1, // backtracking mode
				gcr );
			
			double ibeam, iskydiff, ignddiff;
			double solazi, solzen, solalt, aoi, stilt, sazi, rot, btd;
			int sunup;		

			p_gh[i] = (ssc_number_t)wf.gh;
			p_dn[i] = (ssc_number_t)wf.dn;
			p_df[i] = (ssc_number_t)wf.df;
			p_tamb[i] = (ssc_number_t)wf.tdry;
			p_tdew[i] = (ssc_number_t)wf.tdew;
			p_wspd[i] = (ssc_number_t)wf.wspd;			
			p_tcell[i] = (ssc_number_t)wf.tdry;
	
			int code = irr.calc();

			if ( 0 != code )
				sunup = 0; // if for some reason the irradiance processor fails, ignore this hour
			else
				irr.get_sun( &solazi, &solzen, &solalt, 0, 0, 0, &sunup, 0, 0, 0 );
			
			p_sunup[i] = (ssc_number_t)sunup;

			if (sunup > 0)
			{
				irr.get_angles( &aoi, &stilt, &sazi, &rot, &btd );
				irr.get_poa( &ibeam, &iskydiff, &ignddiff, 0, 0, 0);
				
				if ( sunup > 0 && track_mode == 1
					&& shade_mode_1x == 0 ) // selfshaded mode
				{	
					double shad1xf = shade_fraction_1x( solazi, solzen, tilt, azimuth, gcr, rot );					
					p_shad_beam_factor[i] *= (ssc_number_t)(1-shad1xf);

					if ( fd > 0 && shade_mode_1x == 0 && iskydiff > 0 )
					{
						double reduced_skydiff = iskydiff;
						double Fskydiff = 1.0;
						double reduced_gnddiff = ignddiff;
						double Fgnddiff = 1.0;
						
						// worst-case mask angle using calculated surface tilt
						double phi0 = 180/3.1415926*atan2( sind( stilt ), 1/gcr - cosd( stilt ) );

						// calculate sky and gnd diffuse derate factors
						// based on view factor reductions from self-shading
						diffuse_reduce( solzen, stilt,
							wf.dn, iskydiff+ignddiff,
							gcr, phi0, alb, 1000,

							// outputs (pass by reference)
							reduced_skydiff, Fskydiff,
							reduced_gnddiff, Fgnddiff );

						if ( Fskydiff >= 0 && Fskydiff <= 1 ) iskydiff *= Fskydiff;
						else log( util::format("sky diffuse reduction factor invalid at hour %d: fskydiff=%lg, stilt=%lg", i, Fskydiff, stilt), SSC_NOTICE, (float)i );

						if ( Fgnddiff >= 0 && Fgnddiff <= 1 ) ignddiff *= Fgnddiff;
						else log( util::format("gnd diffuse reduction factor invalid at hour %d: fgnddiff=%lg, stilt=%lg", i, Fgnddiff, stilt), SSC_NOTICE, (float)i );
					}

				}

				// apply hourly shading factors to beam (if none enabled, factors are 1.0)
				ibeam *= p_shad_beam_factor[i];
				
				// apply beam shading based on solar azimuth/altitude table
				if ( enable_azalt_beam_shading )
					ibeam *= util::bilinear( solalt, solazi, azaltvals );
				
				// apply sky diffuse shading factor (specified as constant, nominally 1.0 if disabled in UI)
				iskydiff *= shad_skydiff_factor;
				
				double poa = ibeam + fd*(iskydiff +ignddiff);
				
				double wspd_corr = wf.wspd < 0 ? 0 : wf.wspd;

				if (wind_stow > 0 && wf.wspd >= wind_stow)
					poa = 0;
				
				// module cover
				double tpoa = poa - ( 1.0 - iam( aoi, use_ar_glass ) )*wf.dn*cosd(aoi);
				if( tpoa < 0.0 ) tpoa = 0.0;
			
				// cell temperature
				double pvt = tccalc( poa*concen, wspd_corr, wf.tdry, fhconv );

				// dc power output
				double dc = dc_nameplate*(1.0+gamma*(pvt-t_ref))*tpoa/i_ref;

				// dc losses
				dc *= derate;

				// inverter efficiency
				double etanom = 0.97;
				double A =  -0.0109;
				double B =  -0.0051;
				double C =   0.9888;
				double pdc0 = ac_nameplate/etanom;
				double plr = dc / pdc0;
				double ac = 0;

				if ( plr > 0 )
				{ // normal operation
					double eta = A*plr + B/plr + C;
					ac = dc*eta;
				}

				if ( ac > ac_nameplate ) // clipping
					ac = ac_nameplate;
							
				p_poa[i] = (ssc_number_t)poa;
				p_tpoa[i] = (ssc_number_t)tpoa;
				p_tcell[i] = (ssc_number_t)pvt;
				p_dc[i] = (ssc_number_t)dc;
				p_ac[i] = (ssc_number_t)ac;

			}
		
			i++;
		}

		ssc_number_t *poam = accumulate_monthly( "poa", "poa_monthly", 0.001 );
		accumulate_monthly( "dc", "dc_monthly", 0.001 );
		accumulate_monthly( "ac", "ac_monthly", 0.001 );

		ssc_number_t *solrad = allocate( "solrad_monthly", 12 );
		ssc_number_t solrad_ann = 0;
		for ( int m=0;m<12;m++ )
		{
			solrad[m] = poam[m]/util::nday[m];
			solrad_ann += solrad[m];
		}
		assign( "solrad_annual", var_data( solrad_ann/12 ) );


		accumulate_annual( "ac", "ac_annual", 0.001 );

		assign( "location", var_data( wf.location ) );
		assign( "city", var_data( wf.city ) );
		assign( "state", var_data( wf.state ) );
		assign( "lat", var_data( (ssc_number_t)wf.lat ) );
		assign( "lon", var_data( (ssc_number_t)wf.lon ) );
		assign( "tz", var_data( (ssc_number_t)wf.tz ) );
		assign( "elev", var_data( (ssc_number_t)wf.elev ) );
	}
};

DEFINE_MODULE_ENTRY( pvwatts2014, "PVWatts 2014 - integrated hourly weather reader and PV system simulator.", 2 )
