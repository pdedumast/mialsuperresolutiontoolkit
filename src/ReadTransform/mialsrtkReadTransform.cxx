
/*=========================================================================

Program: Performs super-resolution image reconstruction based on exact Total Variation regularization
Language: C++
Date: $Date: 2015-30-09 $
Version: $Revision: 1 $
Author: $Sebastien Tourbier$

Copyright (c) 2017 Medical Image Analysis Laboratory (MIAL), Lausanne
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

==========================================================================*/


#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

/* Standard includes */
#include <tclap/CmdLine.h>
#include <sstream>

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

// pgdd
#include <typeinfo>
#include <iomanip>
// #include <sstream>


/* Itk includes */
// #include "itkEuler3DTransform.h"
// #include "itkVersorRigid3DTransform.h"
// #include "itkImage.h"
// #include "itkImageFileReader.h"
// #include "itkImageFileWriter.h"
#include "itkImageMaskSpatialObject.h"
#include "itkTransformFileReader.h"
#include "itkTransformFactory.h"
#include "itkCastImageFilter.h"

#include "itkPermuteAxesImageFilter.h"
#include "itkFlipImageFilter.h"
#include "itkOrientImageFilter.h"

/*Btk includes*/
#include "mialsrtkSliceBySliceTransform.h"
//#include "btkSuperResolutionImageFilter.h"
#include "mialsrtkSuperResolutionRigidImageFilterWithImplicitGradientDescent.h"
#include "mialsrtkImageRegistrationFilter.h"

#include "mialsrtkLowToHighImageResolutionMethod.h"

#include "itkImageDuplicator.h"

#include "mialsrtkMaths.h"


int main( int argc, char *argv[] )
{

    try {
        std::vector< std::string > transform;
        std::string outDir;


        // Parse arguments
        TCLAP::CmdLine cmd("Read transformation files () .", ' ', "Unversioned");

        // Input motion parameters - Used only if initHR is disable, meaning that motion parameters were previously estimated
        TCLAP::MultiArg<std::string> transArg("t","in-transform","transform file",false,"string",cmd);

        // Output directory
        TCLAP::ValueArg<std::string> outputDirArg("o","out-dir","Output directory",false,"/resutls/file.txt","string",cmd);


        // Parse the argv array.
        cmd.parse( argc, argv );
        outDir = outputDirArg.getValue();
        transform = transArg.getValue();

        std::cout << std::setprecision(16);
        std::cout << std::endl << "OUTPUT DIRECTORY :: " << outDir << std::endl << std::endl;




        if ( ( transform.size() == 0 ) || ( outDir.size() == 0 ) )
        {
            std::cout << "Missing input argument " << "--in-transform" << " and " << "--out-dir" << std::endl;
            return EXIT_FAILURE;
        }


        // typedefs
        const   unsigned int    Dimension = 3;
        typedef mialsrtk::SliceBySliceTransformBase< double, Dimension > TransformBaseType;
        typedef mialsrtk::SliceBySliceTransform< double, Dimension > TransformType;
        typedef TransformType::Pointer                          TransformPointer;

        // Register the SliceBySlice transform (a non-default ITK transform) with the TransformFactory of ITK
        itk::TransformFactory<TransformType>::RegisterTransform();

        typedef itk::TransformFileReader     TransformReaderType;
        typedef TransformReaderType::TransformListType * TransformListType;


        unsigned int numberOfImages = transform.size();

        std::vector< TransformPointer >     transforms(numberOfImages);




        // Filter setup
        for (unsigned int i=0; i<numberOfImages; i++)
        {
            std::string infile = transform[i];

            std::string base_filename = infile.substr(infile.find_last_of("/\\") + 1);

            std::string::size_type const p(base_filename.find_first_of('.'));
            std::string file_without_extension = base_filename.substr(0, p);


            if (transform.size() > 0 )
            {
                std::cout<<"Reading transform:"<<transform[i]<<std::endl;
                TransformReaderType::Pointer transformReader = TransformReaderType::New();
                transformReader -> SetFileName( transform[i] );
                transformReader -> Update();

                TransformListType transformsList = transformReader->GetTransformList();
                TransformReaderType::TransformListType::const_iterator titr = transformsList->begin();
                TransformPointer trans = static_cast< TransformType * >( titr->GetPointer() );



                for(unsigned int j=0; j< trans -> GetNumberOfSlices(); j++) {


                    std::stringstream ss;
                    ss<<j;

                    std::string s;
                    ss>>s;

                    std::string path = outDir + "/" + file_without_extension + "_" + s + ".txt";
                    std::ofstream fout(path.c_str());
                    fout << std::setprecision(16);

                    std::cout << "   P  A  T  H   " << path << std::endl;

                    for(unsigned int ix=0; ix< 3; ix++) {
                        for(unsigned int iy=0; iy< 3; iy++) {
                            // std::cout << "Type of an element of the inverse matrix: " << typeid(trans->GetSliceTransform(j)->GetMatrix().GetInverse()[ix][iy]).name() << std::endl;
                            fout << trans->GetSliceTransform(j)->GetInverseMatrix()[ix][iy];
                            fout << " ";
                        }
                        fout << std::endl;
                    }


                    fout.close();

                    std::cout << i << " - " << j << " : " << std::endl;
                    std::cout << trans->GetSliceTransform(j)->GetInverseMatrix() << std::endl << std::endl;

                  }
            }
        }



    } catch (TCLAP::ArgException &e)  // catch any exceptions
    { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }

    return EXIT_SUCCESS;
}

