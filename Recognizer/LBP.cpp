#include "LBP.hpp"
#include<ctime>

using namespace lbp;
//clock_t startTime, endTime;
/**
 * Constructors
 */
LBP::LBP( void )
: type( LBP_MAPPING_NONE ), samples( 0 ), num( 0 ) {
}
LBP::LBP( unsigned int _samples, MappingType _type )
: type( _type ), samples( _samples ), num( 0 ) {
	//startTime = clock();	
	generateMapping();
	//endTime = clock();
	//cout << "Generating Table took " << double(endTime - startTime) / double(CLOCKS_PER_SEC) << "s" << endl;
    
//	if( type == LBP_MAPPING_HF ) {
//		initHF();
//	}
}

LBP::LBP(unsigned int _samples)
	: samples(_samples) {
	//cout << "S:  " << samples<< endl;
	if (samples == 8)
		this->num = 10;
	else
		this->num = 18;
	//associateTableMapping(samples);

}

LBP::LBP(unsigned int samples, string mapping) {

	if (samples == 8 && mapping == "u2") {
		num = 59;
		table = &table_8;
	}
	if (samples == 16 && mapping == "u2") {
		num = 243;
		table = &table_16;
	}
	if (samples == 8 && mapping == "riu2"){
		num = 10;
		table = &table_8;
	}

	if (samples == 16 && mapping == "riu2"){
		num = 18;
		table = &table_16;
	}

	//associateTableMapping(samples);

}

LBP::~LBP() {

}

/** ******************************************************************
 *
 * Mapping part
 *
 */
LBP & LBP::generateMapping() {
	return generateMapping( this->samples, this->type );
}

void LBP::associateTableMapping(unsigned int samples, string mapping) {
	//this->orbits.clear();

	//this->table.clear();
	//this->num = 0;
	//this->type = type;
	this->samples = samples;
	if (samples == 8 && mapping == "u2") {
		this->table = &table_8;
		this->num = 59;
	}
	if (samples == 16 && mapping == "u2") 
		{
			this->table = &table_16;
			this->num = 243;
			//cout << "size table  " << table.size() << endl;
		}

		if (samples == 8 && mapping == "riu2") {
			this->table = &table_8;
			this->num = 10;
		}

		if (samples == 16 && mapping == "riu2") {
			this->table = &table_16;
			this->num = 18;
		}

		//	return *this;
	
}

LBP & LBP::generateMapping( unsigned int samples, MappingType type ) {
	this->orbits.clear();
	//this->table.clear();
	this->num = 0;
	this->type = type;
	this->samples = samples;
    
	int newMax = 0; //number of patterns in the resulting LBP code
	int index = 0;
    
	if( type == LBP_MAPPING_NONE ) {
		newMax = (int) pow( 2., (int) samples );
		for( unsigned int i = 0; i < newMax; i++ ) {
			table->push_back(i);
		}
	}
	else if (type == LBP_MAPPING_U2) {
		//cout << "HERE u2" << endl;
		// Uniform 2
		newMax = samples * (samples - 1) + 3;
        
		for( unsigned int i = 0; i < pow( 2., (int) (samples) ); i++ ) {
            
			// Rotate left
            //			unsigned int bg = ((i & (1 << (samples - 1))) >> (samples - 1)); // bitget(i,samples)
            //			unsigned int bs = (i << 1) & ((int) pow( 2., (int) samples ) - 1); // bitshift( i, 1, samples )
            //			unsigned int j = (bs + bg) & ((int) pow( 2., (int) samples ) - 1); // bitset( bs, 1, bg )
			unsigned int j = rotateLeft( i, samples );
            
			int numt = NumberOfSetBits( i ^ j ); // sum(bitget(bitxor(i,j),1:samples));
			//number of 1->0 and 0->1 transitions
			//in binary string
			//x is equal to the
			//number of 1-bits in
			//XOR(x,Rotate left(x))
            
			if( numt <= 2 ) {
				table->push_back( index );
				index = index + 1;
			}
			else {
				table->push_back( newMax - 1 );
			}
		}
	}
	else if( type == LBP_MAPPING_RI ) {
		long N = (int) pow( 2., (int) samples );
		// Rotation Invariant
		int * tmpMap = new int[N];
		memset( (void *)tmpMap, -1, N );
        
		for( unsigned long i = 0; i < N; i++ ) {
			tmpMap[i] = -1;
            
			unsigned long rm = i;
			unsigned long r = i;
			for( int j = 1; j <= samples - 1; j++ ) {
				r = rotateLeft( r, samples );
				if( r < rm )
					rm = r;
			}
			if( tmpMap[rm] < 0 ) {
				tmpMap[rm] = newMax;
				newMax = newMax + 1;
			}
			table->push_back( tmpMap[rm] );
		}
	}
	else if( type == LBP_MAPPING_RIU2 ) {
		// Rotation invariant uniform 2
		newMax = samples + 2;
		for( unsigned int i = 0; i <= pow( 2., (int) samples ) - 1; i++ ) {
			unsigned int j = rotateLeft( i, samples ); //bitset( bitshift( i, 1, samples ), 1, bitget( i, samples ) ); // rotate left
			unsigned int numt = NumberOfSetBits( i ^ j ); //sum(bitget(bitxor(i,j),1:samples));
			if( numt <= 2 )
				table->push_back( NumberOfSetBits( i ) );
			else
				table->push_back( samples + 1 );
		}
	}
	else if( type == LBP_MAPPING_HF ) {
		// Histogram Fourier
		newMax = samples * (samples - 1) + 3;
		table->push_back( newMax - 3 );
        
		for( unsigned int i = 1; i <= pow( 2., (int) samples ) - 2; i++ ) {
			unsigned int j = rotateLeft( i, samples ); // bitset(bitshift(i,1,samples),1,bitget(i,samples)); %rotate left
			unsigned int numt = NumberOfSetBits( i ^ j ); // sum(bitget(bitxor(i,j),1:samples)); %number of 1->0 and 0->1 transitions
            
			if( numt == 2 ) { // Uniform pattern
				unsigned int n = NumberOfSetBits( i ); // sum(bitget(i,1:samples)); %Number of 1-bits
                
				unsigned int bc = j ^ ((unsigned int) pow( 2., (int) samples ) - 1);
				unsigned int ba = bc & i;
				unsigned int f = trailingZeroInd( ba ) + 1; //find(bitget(bitand(i,bitcmp(j,samples)),1:samples)); // Rotation index of the bit pattern
				unsigned int r = ((int) floor( n / 2. ) + f) % samples;
				index = (n - 1) * samples + r;
				table->push_back( index );
			}
			else { // Non-uniform
				table->push_back( newMax - 1 );
			}
		}
        table->push_back( newMax - 2 );
        
		vector<int> o;
		for( int i = 1; i <= samples - 1; i++ ) {
			o.clear();
			for( int j = ((i - 1) * samples); j <= (i * samples - 1); j++ ) {
				o.push_back( j );
			}
			orbits.push_back( o );
		}
		o.clear();
		o.push_back( newMax - 3 );
		orbits.push_back( o );
		o[0] = newMax - 2;
		orbits.push_back( o );
		o[0] = newMax - 1;
		orbits.push_back( o );
	}
	else {
		cerr << "Unknown mapping!" << endl;
		exit(1);
	}
    
	this->num = newMax;

	if (samples == 16)
		table_16 = *table;
	else
		table_8 = *table;

	return *this;
}

/**
 *
 */

bool LBP::saveMapping( string fileName ) {
	ofstream ofs( fileName.c_str(), ios::out );
    if( ! ofs ) {
        cerr << "File \'" << fileName << "\' could not be opened" << endl;
        return false;
    }
    
    ofs << "LBPMapping" << endl;
    ofs << "version " << 1 << endl;
    ofs << "type " << MappingTypeStr[ type ] << endl;
    ofs << "samples " << samples << endl;
    ofs << "maxnum " << num << endl;
    ofs << "table ";
	if (samples == 16) {
		for (int i = 0; i < table->size(); i++) {
			ofs << table_16[i] << " ";
		}
	}
	else {
		for (int i = 0; i < table->size(); i++) 
			ofs << table_8[i] << " ";
		
	}
    ofs << endl;
    if( type == LBP_MAPPING_HF ) {
        ofs << "orbits ";
        for( int i = 0; i < orbits.size(); i++ ) {
            for(int j = 0; j < (orbits[i]).size(); j++ ) {
                ofs << orbits[i][j] << " ";
            }
            ofs << "-1 ";
        }
        ofs << endl;
    }
    
    
	return true;
}


bool LBP::loadMapping( string fileName, int numTable ) {
    ifstream ifs( fileName.c_str(), ios::in );
    if( ! ifs ) {
        cerr << "File \'" << fileName << "\' could not be opened" << endl;
        return false;
    }
    
    
    
    string s; int i;
    // Get file type
    ifs >> s;
    if( s.compare("LBPMapping") ) {
        cerr << fileName << " is not a LBPMapping file." << endl;
        return false;
    }
    
    // Get verion
    ifs >> s >> i;

    // Get mapping type
    ifs >> s >> s;
    this->type = strToType(s);
    
    // Get samples
    ifs >> s >> this->samples;
	//cout << "nummmmmQQQQQQQQQQ" << samples << endl;
    // Get maxnum
    ifs >> s >> this->num;
	//cout << "nummmmmmAAAAAA" << num << endl;
    // Get table
    ifs >> s;

	if (numTable == 2) {
		this->table_16.clear();
		for (int j = 0; j < pow(2., (double)samples); j++) {
			ifs >> i;
			table_16.push_back(i);
		}		//cout << "table_16.size():  " << table_16[5] << endl;
	}
	else if(numTable == 1) {
		this->table_8.clear();
		for (int j = 0; j < pow(2., (double)samples); j++) {
			ifs >> i;
			table_8.push_back(i);
		}  //cout << "table_8.size():  " << table_8[5] << endl;
	}

    if ( type != LBP_MAPPING_HF ) {
        return true;
    }
    // Get orbits for HF
    this->orbits.clear();
    ifs >> s;
    vector<int> o;
    while( ifs >> i ) {
        if( i < 0 ) { // -1 are used as separators
            orbits.push_back(o);
            o.clear();
            continue;
        }
        o.push_back(i);
    }
    
    return true;
}

/** ******************************************************************
 *
 * Descriptor part
 *
 */

LBP & LBP::calcLBP( Mat d_img, int numTable ,double radius, bool borderCopy) {
	if (numTable == 1)
		table = &table_8;
	else
		table = &table_16;

	//cout << "........... " << (*table)[(5)] << endl;
	//cout << "sssssssss "<<table->size() << endl;

    //	clock_t startTime, endTime, sT, eT;
    //	vector<double> times;
    //	double minVal, maxVal;
    //	namedWindow( "lbp", 0 );
    //	Mat dummy( 300, 260, CV_8UC1);

    // Make sure the image has Double precision version
	if( d_img.type() < CV_64F ) {
		d_img.convertTo( d_img, CV_64F );
	}
	
	// Make a copy of the image border the same size as the radius
	if( borderCopy ) {
		
		Mat tmp( d_img.rows+2*radius, d_img.cols+2*radius, CV_64F );
		
        copyMakeBorder( d_img, tmp, radius, radius, radius, radius, BORDER_WRAP, Scalar(0) );
		//system("pause");
		//cout << tmp.size() << endl;
		d_img = tmp.clone();
	}
	//cout << "size image  " << d_img.size() << endl;
	//cout << "samples:  "<<samples << endl;
	//cout << "num:  " << num << endl;

	
	auto spoints = new double[samples][2]();
	double a = 2 * CV_PI / samples;
	double miny = +INT_MAX;
	double maxy = -INT_MAX;
	double minx = +INT_MAX;
	double maxx = -INT_MAX;
	for( int i = 0; i < samples; i++ ) {
		spoints[i][0] = +radius * cos( double( i * a ) );
		spoints[i][1] = -radius * sin( double( i * a ) );
        
		minx = (spoints[i][0] < minx ? spoints[i][0] : minx);
		maxx = (spoints[i][0] > maxx ? spoints[i][0] : maxx);
		miny = (spoints[i][1] < miny ? spoints[i][1] : miny);
		maxy = (spoints[i][1] > maxy ? spoints[i][1] : maxy);
        
	}
    
	// Determine the dimensions of the input image.
	int xsize = d_img.cols;
	int ysize = d_img.rows;
    
	// Block size, each LBP code is computed within a block of size bsizey*bsizex
	int bsizex = ceil( max( maxx, 0. ) ) - floor( min( minx, 0. ) ) + 1;
	int bsizey = ceil( max( maxy, 0. ) ) - floor( min( miny, 0. ) ) + 1;
    
	// Minimum allowed size for the input image depends
	// on the radius of the used LBP operator.
	if( xsize < bsizex || ysize < bsizey ) {
		cerr << "Too small input image. Should be at least (2*radius+1) x (2*radius+1)" << endl;
		return *this;
	}
    
	// Coordinates of origin (0,0) in the block
	int origx = 1 - floor( min( minx, 0. ) ) - 1;
	int origy = 1 - floor( min( miny, 0. ) ) - 1;
	
	// Calculate dx and dy;
	int dx = xsize - bsizex + 1;
	int dy = ysize - bsizey + 1;
	//cout << "dy:  " << dy << endl;  cout << "dy:  " << dx << endl;
	// Fill the center pixel matrix C.
	// d_C is a shallow copie. But that's OK because we are not changing the values
	//	but only comparing to N
	Mat d_C( d_img, Rect( origx, origy, dx, dy ) );
    
	// Initialize the result matrix with zeros.
	//Mat result(dy, dx, CV_64FC1); 
	Mat result = Mat::zeros( dy, dx, CV_64FC1 ); 
	Mat D( dy, dx, CV_64FC1);
	Mat N( dy, dx, CV_64FC1);
	
	// Compute the LBP code image
    //	startTime = clock();
	for( int i = 0; i < samples; i++ ) {
		double x = spoints[i][0] + origx;
		double y = spoints[i][1] + origy;
		// Calculate floors, ceils and rounds for the x and y.
		int fy = floor( y );
		int cy = ceil( y );
		int ry = round( y );
		int fx = floor( x );
		int cx = ceil( x );
		int rx = round( x );

		// Check if interpolation is needed.
		if( (fabs( x - rx ) < 1e-6) && (fabs( y - ry ) < 1e-6) ) {
			// Interpolation is not needed, use original data types
			// N is a shallow copy. But that's OK because we are not changing the value
			//	but only comparing to C
			Mat N( d_img, Rect( rx, ry, dx, dy ) );
			compare( N, d_C, D, CMP_GE ); // D = (N >= C);
		}
		else {
			//cout << "INTERPOLATION NEDDED" << endl;
			// Interpolation needed, use double type images
			double tx = x - fx;
			double ty = y - fy;
            
			// Calculate the interpolation weights.
			double w1 = (1 - tx) * (1 - ty);
			double w2 = tx * (1 - ty);
			double w3 = (1 - tx) * ty;
			double w4 = tx * ty;

			// Compute interpolated pixel values
            //			N = w1 * d_img( Rect( fx, fy, dx, dy ) ) + w2 * d_img( Rect( cx, fy, dx, dy ) )
            //						+ w3 * d_img( Rect( fx, cy, dx, dy ) )
            //						+ w4 * d_img( Rect( cx, cy, dx, dy ) );
			// The below operations are about 20% faster than the above
			addWeighted( d_img( Rect( fx, fy, dx, dy ) ), w1,
						 d_img( Rect( cx, fy, dx, dy ) ), w2, 0, N );
			addWeighted( d_img( Rect( fx, cy, dx, dy ) ), w3, N, 1, 0, N );
			addWeighted( d_img( Rect( cx, cy, dx, dy ) ), w4, N, 1, 0, N );
            
			compare( N, d_C, D, CMP_GE ); // D = (N >= C);
		}


		// Update the result matrix.
		double v = pow( 2., i ) / 255.; // Divide by 255 because D is 0/255 rather than 0/1

		D.convertTo( D, CV_64F );
		result = result + (v * D);
		
	}
	
    //	endTime = clock();
    //	times.push_back( (endTime - startTime) );
    //	cout << "lbp calc took " << times.back() << " cycles" << endl;

	//cout << "table size:  " << table.size() << endl;
    //	startTime = clock();
	// Apply mapping if it is defined
	if( type != LBP_MAPPING_NONE ) {
		MatIterator_<double> it = result.begin<double>(), it_end = result.end<double>();
		for( ; it != it_end; ++it ) {
			*it = (*table)[(*it)];
		}
	}
	result.convertTo(result, CV_8UC1);
    //	endTime = clock();
    //	times.push_back( (endTime - startTime) );
    //	cout << "mapping took " << times.back() << " cycles" << endl;
    
	// Store the final result
	lbpImage = result.clone();
	#if 0
    for( int j = 0; j < lbpImage.rows; j++ ) {
		for( int i = 0; i < lbpImage.cols; i++ ) {
			//cout.width(3);
			cout << int(lbpImage.at<unsigned char>(j,i)) << " ";
		}
		cout << endl;
	}
	#endif
	return *this;
}

bool LBP::saveLBPImage( string fileName ) {
	return cv::imwrite( fileName, this->lbpImage );
}
/** ******************************************************************
 *
 * Histogram part
 *
 */
LBP & LBP::calcHist( void ) {
	return calcHist( &lbpImage );
}
LBP & LBP::calcHist( Mat mask ) {
	return calcHist( &lbpImage, &mask );
}
LBP & LBP::calcHist( Mat * lbpImg, Mat * mask ) {
	
	if (lbpImg == NULL) {
		lbpImg = &(this->lbpImage);
	}
	
	int histSize = num; //cout << "NUMMMMMMM:  " << num << endl; 
	float range[] = { 0, num };
	const float* histRange = { range };
	if( mask == NULL ) {
		cv::calcHist( lbpImg, 1, 0, Mat(), // do not use mask
                     hist, 1, &histSize, &histRange, true, // the histogram is uniform
                     false // do not accumulate
                     );
	}
	else {
		cv::calcHist( lbpImg, 1, 0, *mask, // use mask
                     hist, 1, &histSize, &histRange, true, // the histogram is uniform
                     false // do not accumulate
                     );
	}
	return *this;
}

vector<double> LBP::getHist( bool norm ) {
	vector<double> h( hist.rows );
	Scalar sum( 1 );
    
	// normalization value
	if( norm || type == LBP_MAPPING_HF ) {
		sum = cv::sum( hist );
	}
	float * k = &hist.at<float>(0);
	for( int i = 0; i < hist.rows; i++ ) {
		h[i] = k[i] / sum[0];
	}
	
//	if( type == LBP_MAPPING_HF ) {
//		h = constructHF( h );
//	}
    
	return h;
}
/*
void LBP::initHF( void ) {
	// All the vectors in the orbit have at least the same size as the first one.
	//	only the last 3 are off size 1 which are not converted anyway
	fftN = this->orbits[0].size();
	// Since the input data are real we take advantage of Hermetian redundancy. This
	// 	gives us a speed up
	fftHermN = floor( fftN / 2 ) + 1;
    
	// If the size of this fft array is different from previous
	if( fftN != fftN && fftIn != NULL ) {
		//fftw_free( fftIn );
		delete[] fftOut;
		fftIn = NULL;
		fftOut = NULL;
	}
	// If the in/out arrays are uninitialized
	if( fftIn == NULL ) {
		//fftIn = (double *) fftw_malloc( sizeof(double) * fftN );
		fftOut = new complex<double> [fftHermN];
	}
	// Setup the fft plan
	//fftPlan = fftw_plan_dft_r2c_1d(
      //                             fftN, fftIn, reinterpret_cast<fftw_complex *>( fftOut ), FFTW_ESTIMATE);
    
}

vector<double> LBP::constructHF( vector<double> h ) {
	if( this->type != LBP_MAPPING_HF ) {
		cerr << "The mapping type must be " << MappingTypeStr[LBP_MAPPING_HF] << endl;
		return h;
	}
    
	initHF();
    
	// Size of the output vector
    //	int FVLEN = (samples - 1) * (floor( samples / 2 ) + 1) + 3;
    //	hf.reserve( FVLEN );
	hf.clear();
    
	for( int j = 0; j < this->orbits.size(); j++ ) {
		if( orbits[j].size() > 1 ) {
			// transfer in the data
			for( int i = 0; i < fftN; i++ ) {
				fftIn[i] = h[orbits[j][i]];
			}
            
			//fftw_execute( fftPlan );
            
			// read out the data
			for( int i = 0; i < fftHermN; i++ ) {
				hf.push_back( abs( fftOut[i] ) );
			}
		}
		else {
			hf.push_back( h[orbits[j][0]] );
		}
	}
    
	return hf;
}
*/

/** ******************************************************************
 *
 * Others part
 *
 */
/*
std::string LBP::toString( bool verbose ) const {
	string s = "LBP = \n";
	s += "\t    type: " + MappingTypeStr[type] + "\n";
    if( verbose ) {
        s += "\t   table: [";
        for( int i = 0; i < table.size(); i++ )
            s += SSTR( table[i] ) + (i < table.size()-1 ? ", " : "");
        s += "]\n";
    }
    else {
        s += "\t   table: [1x" + SSTR( pow( 2., (int) this->samples ) )+ (string) "]\n";
    }
	s += "\t samples: " + SSTR( this->samples )+ (string) "\n";
	s += "\t     num: " + SSTR( this->num )+ (string) "\n";
	if( this->type == LBP_MAPPING_HF ) {
        if (verbose) {
            s += "\t  orbits: {";
            for (int i = 0; i < orbits.size(); i++ ) {
                s += "{";
                for (int j = 0; j < orbits[i].size(); j++) {
                    s += SSTR( orbits[i][j]) + (j < orbits[i].size()-1 ? ", " : "");
                }
                s += (string)"}" + (i < orbits.size()-1 ? ", " : "");
            }
            s += "}\n";
        }
        else {
            s += "\t  orbits: {" + SSTR( this->orbits.size() )+ (string) "x1}\n";
        }
    }
	return s;
}
*/
