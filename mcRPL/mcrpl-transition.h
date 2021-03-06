#ifndef MCRPL_TRANSITION_H
#define MCRPL_TRANSITION_H
//#include"CuEnvControl.h"
//#include "errorhelper.h"
#include "mcrpl-basicTypes.h"
#include "mcrpl-cellspace.h"
#include "mcrpl-neighborhood.h"
#include "mcrpl-subCellspace.h"
//#include"CuPRL.h"
//#include "FocalCodeOperator.h"
//#include"mcRPL-transition.h"
#include "mcrpl-OperatorDevice.h"
#include "mcrpl-LocalOperatorDevice.h"
#include "mcrpl-CuPRL.h"
namespace mcRPL {
    
  class Transition {
    public:
      /* Constructor and destructor */
      Transition(bool onlyUpdtCtrCell = true,
                 bool needExchange = false,
                 bool edgesFirst = false);
      virtual ~Transition() {}

      /* Layers */
      void addInputLyr(const char *aInLyrName,
                       bool isPrimeLyr = false);
	  void addparam(double dparam);                       //2018
	  template<class T>
	  void addparam(vector<T>vPara);
	  void clearGPUMem(); 
      void addOutputLyr(const char *aOutLyrName,
                        bool isPrimeLyr = true);
      bool setLyrsByNames(vector<string> *pvInLyrNames,
                          vector<string> *pvOutLyrNames,
                          string &primeLyrName);
      const vector<string>& getInLyrNames() const;
      const vector<string>& getOutLyrNames() const;
      const string& getPrimeLyrName() const;
      bool isInLyr(const string &lyrName) const;
      bool isOutLyr(const string &lyrName) const;
      bool isPrimeLyr(const string &lyrName) const;
      void clearLyrSettings();
	  void setSMcount(int _sm);
      bool setCellspace(const string &lyrName,
                        mcRPL::Cellspace *pCellspc);
      void clearCellspaces();
	  bool initGlobalCoords(const vector<mcRPL::CellCoord> &vGlobalCoords);
	  bool initGlobalCoords(const vector<int> &vGlobalCoords);
	   void clearGlobalCoords();
      mcRPL::Cellspace* getCellspaceByLyrName(const string &lyrName);
      const mcRPL::Cellspace* getCellspaceByLyrName(const string &lyrName) const;

      void setUpdateTracking(bool toTrack);
      void clearUpdateTracks();

      /* Neighborhood */
      void setNbrhdByName(const char *aNbrhdName);
      const string& getNbrhdName() const;
      void clearNbrhdSetting();

      void setNbrhd(Neighborhood *pNbrhd = NULL);

      mcRPL::Neighborhood* getNbrhd();
      const mcRPL::Neighborhood* getNbrhd() const;

      void clearDataSettings();

      /* Option information */
      bool onlyUpdtCtrCell() const;
      bool needExchange() const;
      bool edgesFirst() const;
      /* Collective Evaluation  */
  public:
	   template<class DataInType, class DataOutType, class OperType>
	   mcRPL::EvaluateReturn cuFocalOperator(const mcRPL::CoordBR &br);
	   template<class OperType>
	   mcRPL::EvaluateReturn cuFocalMutiOperator(const mcRPL::CoordBR &br);
	   template<class OperType>
	   mcRPL::EvaluateReturn cuLocalOperator(const mcRPL::CoordBR &br);
	   template<class OperType>
	   mcRPL::EvaluateReturn cuZonelOperator(const mcRPL::CoordBR &br);
	    template<class OperType>
	   mcRPL::EvaluateReturn cuGlobalOperator(const mcRPL::CoordBR &br);
  public:
      mcRPL::EvaluateReturn evalBR(const mcRPL::CoordBR &br,bool isGPUCompute=NULL, mcRPL::pCuf pf=NULL);
      mcRPL::EvaluateReturn evalRandomly(const mcRPL::CoordBR &br);
      mcRPL::EvaluateReturn evalSelected(const mcRPL::CoordBR &br,
                                        const mcRPL::LongVect &vlclIdxs);

      /* ------------------------------------------------- *
       *             User-defined Processing               *
       * ------------------------------------------------- */

      /* After setting the Cellspaces for the Transition,
       * if the Cellspaces are SubCellspaces, the global ID of the SubCellspaces
       * will be given to this function.
       * The Transition may use this information to set other user-defined options
       * before running.
       * */
      virtual bool afterSetCellspaces(int subCellspcGlbIdx = mcRPL::ERROR_ID);

      /* After setting the Neighborhood for the Transition,
       * the Transition may set other user-defined options before running
       * */
      virtual bool afterSetNbrhd();

      /* Final check before running */
      virtual bool check() const;

      /* Evaluate a specific Cell */
      virtual mcRPL::EvaluateReturn evaluate(const mcRPL::CellCoord &coord);

      /* Calculate the workload */
      virtual double workload(const mcRPL::CoordBR &workBR) const;

    protected:
      string _primeLyrName;
      vector<string> _vInLyrNames;
      vector<string> _vOutLyrNames;
      map<string, mcRPL::Cellspace*> _mpCellspcs;
	  int _smCount;
      string _nbrhdName;
      mcRPL::Neighborhood *_pNbrhd;
	  vector<double> _vparamInfo;                                                   //2018/10/18
	  vector<int>_globalCoords;
      bool _onlyUpdtCtrCell;
      bool _needExchange;
      bool _edgesFirst;
  };
 template<class T>
 void mcRPL::Transition::addparam(vector<T>vPara)
 {
	 int nSize=vPara.size();
	 for(int i=0;i<nSize;i++)
	 {
	 this->_vparamInfo.push_back((double)vPara[i]);
	 }
 }
 template<class OperType>
  mcRPL::EvaluateReturn mcRPL::Transition::cuFocalMutiOperator(const mcRPL::CoordBR &br)
  {
	//  vector<mcRPL::WeightedCellCoord>coords =_pNbrhd->getInnerNbr(); 
	  int nbrsize =_pNbrhd->size();
	  double *d_weights;
	  vector<int>cuNbrCoords=_pNbrhd->cuCoords();
	  vector<double>cuNbrWeights=_pNbrhd->cuWeights();
	  int brminIRow=br.minIRow();
	  int brmaxIRow=br.maxIRow();
	  int brminICol=br.minICol();
	  int brmaxICol=br.maxICol();

	  //for(int iNbr = 0; iNbr < nbrsize; iNbr++)
	  //{
		 // /* if(coords[iNbr].iRow() == 0 && coords[iNbr].iCol() == 0 ) 
		 // {
		 // continue;
		 // }*/
		 // cuNbrCoords.push_back(coords[iNbr].iRow());
		 // cuNbrCoords.push_back(coords[iNbr].iCol());
		 // cuNbrWeights.push_back(coords[iNbr].weight());
	  //}
	  int *cucoords = &cuNbrCoords[0];
	  double *weights=&cuNbrWeights[0];
	  mcRPL::Cellspace *pRrmSpc=getCellspaceByLyrName(getPrimeLyrName());
	  double cellWidth =fabs(pRrmSpc->info()->georeference()->cellSize().x());
	  double cellHeight =fabs(pRrmSpc->info()->georeference()->cellSize().y());
	  int nwidth=pRrmSpc->info()->dims().nCols();
	  int nheight=pRrmSpc->info()->dims().nRows();
	  int numInRaster=_vInLyrNames.size();
	  int numOutRaster=_vOutLyrNames.size();
	  void **pDataIn=new void *[numInRaster];
	  void **pDataOut=new void *[numOutRaster];
	  int *pDataInType=new int[numInRaster];
	  int *pDataOutType=new int[numOutRaster];
	  void **d_pDataIn;
	  void **d_pDataOut;
	  int *d_pDataInType;
	  int *d_pDataOutType;
	  int *d_nbrcoord;
	  void **outdata=new void *[numOutRaster];
	  double* d_paramInfo;
	   if(_vparamInfo.size()!=0)
	  {
		  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)*_vparamInfo.size()));
		  checkCudaErrors(cudaMemcpy(d_paramInfo,&_vparamInfo[0],sizeof(double)*_vparamInfo.size(),cudaMemcpyHostToDevice));
	  }
	  else
	  {
		  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)));
	  }
	  checkCudaErrors(cudaMalloc(&d_nbrcoord, sizeof(int)*nbrsize * 2));
	  checkCudaErrors(cudaMalloc(&d_weights, sizeof(double)*nbrsize));
	 // checkCudaErrors(cudaMalloc(&d_Types, 40*numInRaster));
	  for(int i=0;i<numInRaster;i++)
	  {

		//  cout<<strlen(pRrmSpc->info()->dataType())+1<<endl;
		  pRrmSpc=getCellspaceByLyrName(getInLyrNames()[i]);
		//  const char *strDataType=pRrmSpc->info()->dataType();
		//  checkCudaErrors(cudaMalloc((void**)&pDataIn[i],nwidth*nheight*pRrmSpc->info()->dataSize()));
		   //checkCudaErrors(cudaMalloc((void**)&pDataInType[i],strlen(pRrmSpc->info()->dataType())+1));
		//  checkCudaErrors(cudaMemcpy(pDataIn[i],pRrmSpc->getData<void>(),nwidth*nheight*pRrmSpc->info()->dataSize(),cudaMemcpyHostToDevice));
		   pDataIn[i]=pRrmSpc->getGPUData();
		   pDataInType[i]=pRrmSpc->info()->cudaTypeCopy();
	//	   cout<< pDataInType[i]<<endl;
		   //checkCudaErrors(cudaMemcpy(pDataInType[i],pRrmSpc->info()->dataType(),strlen(pRrmSpc->info()->dataType())+1,cudaMemcpyHostToDevice));
	  }
	  for(int i=0;i<numOutRaster;i++)
	  {
		  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		  outdata[i]=new char[nwidth*nheight*pRrmSpc->info()->dataSize()];
		//  checkCudaErrors(cudaMalloc((void**)&pDataOut[i],nwidth*nheight*pRrmSpc->info()->dataSize()));
		  pDataOut[i]=pRrmSpc->getGPUData();
		 // checkCudaErrors(cudaMalloc((void**)&pDataOutType[i],strlen(pRrmSpc->info()->dataType())+1));
		 // checkCudaErrors(cudaMemcpy(pDataOutType[i],pRrmSpc->info()->dataType(),strlen(pRrmSpc->info()->dataType())+1,cudaMemcpyHostToDevice));
		  pDataOutType[i]=pRrmSpc->info()->cudaTypeCopy();
	//	  cout<< pDataOutType[i]<<endl;
	  }
	  checkCudaErrors(cudaMalloc(&d_pDataInType,sizeof(int)*numInRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataInType,pDataInType,sizeof(int)*numInRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMalloc(&d_pDataOutType,sizeof(int)*numOutRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataOutType,pDataOutType,sizeof(int)*numOutRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMalloc((void***)&d_pDataIn,sizeof(void*)*numInRaster));
	  checkCudaErrors(cudaMalloc((void***)&d_pDataOut,sizeof(void*)*numOutRaster));
	 //  checkCudaErrors(cudaMalloc((void***)&d_pDataInType,sizeof(void*)*numInRaster));
	 //   checkCudaErrors(cudaMalloc((void***)&d_pDataOutType,sizeof(void*)*numOutRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataIn,pDataIn,sizeof(void*)*numInRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMemcpy(d_pDataOut,pDataOut,sizeof(void*)*numOutRaster,cudaMemcpyHostToDevice));
	 // checkCudaErrors(cudaMemcpy(d_pDataInType,pDataInType,sizeof(void*)*numInRaster,cudaMemcpyHostToDevice));
	 // checkCudaErrors(cudaMemcpy(d_pDataOutType,pDataOutType,sizeof(void*)*numOutRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMemcpy(d_nbrcoord,cucoords, sizeof(int)*nbrsize * 2, cudaMemcpyHostToDevice));
	  checkCudaErrors( cudaMemcpy(d_weights, weights, sizeof(double)*nbrsize, cudaMemcpyHostToDevice));
	 // dim3 block(16,16);
	 // dim3 grid(nwidth% 16 == 0 ? nwidth /16 : nwidth / 16 + 1, nheight % 16 == 0 ? nheight /16 : nheight /16 + 1);
	  int block=256;
	  int gird=_smCount*24;
	  G_FocalMutiOperator<OperType> <<< gird,block>>> (d_pDataIn,d_pDataInType, d_pDataOut, d_pDataOutType, brminIRow, brmaxIRow,brminICol, brmaxICol,nwidth, nheight,d_nbrcoord,d_weights,   nbrsize, cellWidth, cellHeight,d_paramInfo, OperType());
	  cudaDeviceSynchronize();
	  
	  for(int i=0;i<numOutRaster;i++)
	  {

		  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		  checkCudaErrors(cudaMemcpy(outdata[i],pDataOut[i],nwidth*nheight*pRrmSpc->info()->dataSize(),cudaMemcpyDeviceToHost));
		//  cout<<"�ɹ�"<<endl;
		  pRrmSpc->brupdateCell(br,outdata[i]);
		 // for(long iRow = br.minIRow(); iRow <= br.maxIRow(); iRow++) 
		 // {
			//  for(long iCol = br.minICol(); iCol <= br.maxICol(); iCol++) 
			//  {
			//	  /* done = evaluate(mcRPL::CellCoord(iRow, iCol));
			//	  if(done == mcRPL::EVAL_FAILED ||
			//	  done == mcRPL::EVAL_TERMINATED) {
			//	  return done;
			//	  }*/
			////	  cout<<((DataOutType*)outdata[i])[iRow*nwidth+iCol]<<" ";
			//	//  cout<<(DataOutType*)outdata[i])[iRow*nwidth+iCol]<<" ";
			//	  pRrmSpc->updateCellAs<DataOutType>(mcRPL::CellCoord(iRow, iCol),((DataOutType*)outdata[i])[iRow*nwidth+iCol], true);
			//  }
		 // }
	  }
	  for(int i=0;i<numOutRaster;i++)
	  {
		  delete []outdata[i];
		//  getCellspaceByLyrName(getOutLyrNames()[i])->deleteGPUDATA();
		//  cudaFree(pDataOut[i]);
		  // cudaFree(pDataOutType);
	  }
	  for(int i=0;i<numInRaster;i++)
	  {
		 // cudaFree(pDataIn[i]);
		   //getCellspaceByLyrName(getInLyrNames()[i])->deleteGPUDATA();
		 // cudaFree(pDataInType[i]);
	  }
	  cudaFree(d_pDataInType);
	  cudaFree(d_pDataOutType);
	  checkCudaErrors(cudaFree(d_paramInfo));
	  cudaFree(d_weights);
	  cudaFree(d_nbrcoord);
	  return mcRPL::EVAL_SUCCEEDED;
  }

  template<class OperType>
  mcRPL::EvaluateReturn mcRPL::Transition::cuGlobalOperator(const mcRPL::CoordBR &br)
  {
	  int brminIRow=br.minIRow();
	  int brmaxIRow=br.maxIRow();
	  int brminICol=br.minICol();
	  int brmaxICol=br.maxICol();

	  //for(int iNbr = 0; iNbr < nbrsize; iNbr++)
	  //{
		 // /* if(coords[iNbr].iRow() == 0 && coords[iNbr].iCol() == 0 ) 
		 // {
		 // continue;
		 // }*/
		 // cuNbrCoords.push_back(coords[iNbr].iRow());
		 // cuNbrCoords.push_back(coords[iNbr].iCol());
		 // cuNbrWeights.push_back(coords[iNbr].weight());
	  //}
	  mcRPL::Cellspace *pRrmSpc=getCellspaceByLyrName(getPrimeLyrName());
	  double cellWidth =fabs(pRrmSpc->info()->georeference()->cellSize().x());
	  double cellHeight =fabs(pRrmSpc->info()->georeference()->cellSize().y());
	  int nwidth=pRrmSpc->info()->dims().nCols();
	  int nheight=pRrmSpc->info()->dims().nRows();
	  int numInRaster=_vInLyrNames.size();
	  int numOutRaster=_vOutLyrNames.size();
	  void **pDataIn=new void *[numInRaster];
	  void **pDataOut=new void *[numOutRaster];
	  int *pDataInType=new int[numInRaster];
	  int *pDataOutType=new int[numOutRaster];
	  void **d_pDataIn;
	  void **d_pDataOut;
	  int *d_pDataInType;
	  int *d_pDataOutType;
	  void **outdata=new void *[numOutRaster];
	  double* d_paramInfo;
	  int *d_globalcoord;
	   if(_vparamInfo.size()!=0)
	  {
		  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)*_vparamInfo.size()));
		  checkCudaErrors(cudaMemcpy(d_paramInfo,&_vparamInfo[0],sizeof(double)*_vparamInfo.size(),cudaMemcpyHostToDevice));
	  }
	  else
	  {
		  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)));
	  }
	   if(_globalCoords.size()!=0)
	   {
		   checkCudaErrors(cudaMalloc(&d_globalcoord,sizeof(int)*_globalCoords.size()));
		  checkCudaErrors(cudaMemcpy(d_globalcoord,&_globalCoords[0],sizeof(int)*_globalCoords.size(),cudaMemcpyHostToDevice));
	   }
	   else
	   {
		   checkCudaErrors(cudaMalloc(&d_globalcoord,sizeof(int)));
	   }
	 // checkCudaErrors(cudaMalloc(&d_Types, 40*numInRaster));
	  for(int i=0;i<numInRaster;i++)
	  {

		//  cout<<strlen(pRrmSpc->info()->dataType())+1<<endl;
		  pRrmSpc=getCellspaceByLyrName(getInLyrNames()[i]);
		//  const char *strDataType=pRrmSpc->info()->dataType();
		//  checkCudaErrors(cudaMalloc((void**)&pDataIn[i],nwidth*nheight*pRrmSpc->info()->dataSize()));
		   //checkCudaErrors(cudaMalloc((void**)&pDataInType[i],strlen(pRrmSpc->info()->dataType())+1));
		//  checkCudaErrors(cudaMemcpy(pDataIn[i],pRrmSpc->getData<void>(),nwidth*nheight*pRrmSpc->info()->dataSize(),cudaMemcpyHostToDevice));
		   pDataIn[i]=pRrmSpc->getGPUData();
		   pDataInType[i]=pRrmSpc->info()->cudaTypeCopy();
	//	   cout<< pDataInType[i]<<endl;
		   //checkCudaErrors(cudaMemcpy(pDataInType[i],pRrmSpc->info()->dataType(),strlen(pRrmSpc->info()->dataType())+1,cudaMemcpyHostToDevice));
	  }
	  for(int i=0;i<numOutRaster;i++)
	  {
		  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		  outdata[i]=new char[nwidth*nheight*pRrmSpc->info()->dataSize()];
		//  checkCudaErrors(cudaMalloc((void**)&pDataOut[i],nwidth*nheight*pRrmSpc->info()->dataSize()));
		  pDataOut[i]=pRrmSpc->getGPUData();
		 // checkCudaErrors(cudaMalloc((void**)&pDataOutType[i],strlen(pRrmSpc->info()->dataType())+1));
		 // checkCudaErrors(cudaMemcpy(pDataOutType[i],pRrmSpc->info()->dataType(),strlen(pRrmSpc->info()->dataType())+1,cudaMemcpyHostToDevice));
		  pDataOutType[i]=pRrmSpc->info()->cudaTypeCopy();
	//	  cout<< pDataOutType[i]<<endl;
	  }
	  checkCudaErrors(cudaMalloc(&d_pDataInType,sizeof(int)*numInRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataInType,pDataInType,sizeof(int)*numInRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMalloc(&d_pDataOutType,sizeof(int)*numOutRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataOutType,pDataOutType,sizeof(int)*numOutRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMalloc((void***)&d_pDataIn,sizeof(void*)*numInRaster));
	  checkCudaErrors(cudaMalloc((void***)&d_pDataOut,sizeof(void*)*numOutRaster));
	 //  checkCudaErrors(cudaMalloc((void***)&d_pDataInType,sizeof(void*)*numInRaster));
	 //   checkCudaErrors(cudaMalloc((void***)&d_pDataOutType,sizeof(void*)*numOutRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataIn,pDataIn,sizeof(void*)*numInRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMemcpy(d_pDataOut,pDataOut,sizeof(void*)*numOutRaster,cudaMemcpyHostToDevice));
	  int block=256;
	  int gird=_smCount*24;                                      
	  G_GlobalMutiOperator<OperType> <<< gird,block>>> (d_pDataIn,d_pDataInType, d_pDataOut, d_pDataOutType, brminIRow, brmaxIRow,brminICol, brmaxICol,nwidth, nheight, cellWidth, cellHeight,d_paramInfo, d_globalcoord,OperType());
	  cudaDeviceSynchronize();
	  
	  for(int i=0;i<numOutRaster;i++)
	  {

		  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		  checkCudaErrors(cudaMemcpy(outdata[i],pDataOut[i],nwidth*nheight*pRrmSpc->info()->dataSize(),cudaMemcpyDeviceToHost));
		//  cout<<"�ɹ�"<<endl;
		  pRrmSpc->brupdateCell(br,outdata[i]);
		 // for(long iRow = br.minIRow(); iRow <= br.maxIRow(); iRow++) 
		 // {
			//  for(long iCol = br.minICol(); iCol <= br.maxICol(); iCol++) 
			//  {
			//	  /* done = evaluate(mcRPL::CellCoord(iRow, iCol));
			//	  if(done == mcRPL::EVAL_FAILED ||
			//	  done == mcRPL::EVAL_TERMINATED) {
			//	  return done;
			//	  }*/
			////	  cout<<((DataOutType*)outdata[i])[iRow*nwidth+iCol]<<" ";
			//	//  cout<<(DataOutType*)outdata[i])[iRow*nwidth+iCol]<<" ";
			//	  pRrmSpc->updateCellAs<DataOutType>(mcRPL::CellCoord(iRow, iCol),((DataOutType*)outdata[i])[iRow*nwidth+iCol], true);
			//  }
		 // }
	  }
	  for(int i=0;i<numOutRaster;i++)
	  {
		  delete []outdata[i];
		//  getCellspaceByLyrName(getOutLyrNames()[i])->deleteGPUDATA();
		//  cudaFree(pDataOut[i]);
		  // cudaFree(pDataOutType);
	  }
	  for(int i=0;i<numInRaster;i++)
	  {
		 // cudaFree(pDataIn[i]);
		   //getCellspaceByLyrName(getInLyrNames()[i])->deleteGPUDATA();
		 // cudaFree(pDataInType[i]);
	  }
	  cudaFree(d_pDataInType);
	  cudaFree(d_pDataOutType);
	  checkCudaErrors(cudaFree(d_paramInfo));
	  cudaFree(d_globalcoord);
	  return mcRPL::EVAL_SUCCEEDED;

  }

  //2018/10/18
  template<class OperType>
  mcRPL::EvaluateReturn mcRPL::Transition::cuZonelOperator(const mcRPL::CoordBR &br)
  {
	 int brminIRow=br.minIRow();
	  int brmaxIRow=br.maxIRow();
	  int brminICol=br.minICol();
	  int brmaxICol=br.maxICol();
	  mcRPL::Cellspace *pRrmSpc=getCellspaceByLyrName(getPrimeLyrName());
	  int nwidth=pRrmSpc->info()->dims().nCols();
	  int nheight=pRrmSpc->info()->dims().nRows();
	  if(brminIRow!=0||brminICol!=0||brmaxIRow!=(nheight-1)|| brmaxICol!=(nwidth-1))
	  {
		  cout<<"you has defined a nrb,please use FocalOperator";
		  return mcRPL::EVAL_FAILED;
	  }
	 // int *cucoords = &cuNbrCoords[0];
	//  double *weights=&cuNbrWeights[0];
	  double cellWidth =fabs(pRrmSpc->info()->georeference()->cellSize().x());
	  double cellHeight =fabs(pRrmSpc->info()->georeference()->cellSize().y());
	  int numInRaster=_vInLyrNames.size();
	  int numOutRaster=_vOutLyrNames.size();
	  void **pDataIn=new void *[numInRaster];
	  void **pDataOut=new void *[numOutRaster];
	  int *pDataInType=new int[numInRaster];
	  int *pDataOutType=new int[numOutRaster];
	  void **d_pDataIn;
	  void **d_pDataOut;
	  int *d_pDataInType;
	  int *d_pDataOutType;
	//  int *d_nbrcoord;
	  void **outdata=new void *[numOutRaster];
	  double* d_paramInfo;
	   if(_vparamInfo.size()!=0)
	  {
		  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)*_vparamInfo.size()));
		  checkCudaErrors(cudaMemcpy(d_paramInfo,&_vparamInfo[0],sizeof(double)*_vparamInfo.size(),cudaMemcpyHostToDevice));
	  }
	  else
	  {
		  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)));
	  }
	 // checkCudaErrors(cudaMalloc(&d_nbrcoord, sizeof(int)*nbrsize * 2));
	  //checkCudaErrors(cudaMalloc(&d_weights, sizeof(double)*nbrsize));
	  for(int i=0;i<numInRaster;i++)
	  {
		  pRrmSpc=getCellspaceByLyrName(getInLyrNames()[i]);
		   pDataIn[i]=pRrmSpc->getGPUData();
		   pDataInType[i]=pRrmSpc->info()->cudaTypeCopy();
	//	   cout<< pDataInType[i]<<endl;
	  }
	  for(int i=0;i<numOutRaster;i++)
	  {
		  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		  outdata[i]=new char[nwidth*nheight*pRrmSpc->info()->dataSize()];
		  pDataOut[i]=pRrmSpc->getGPUData();
		  pDataOutType[i]=pRrmSpc->info()->cudaTypeCopy();
	//	  cout<< pDataOutType[i]<<endl;
	  }
	  checkCudaErrors(cudaMalloc(&d_pDataInType,sizeof(int)*numInRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataInType,pDataInType,sizeof(int)*numInRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMalloc(&d_pDataOutType,sizeof(int)*numOutRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataOutType,pDataOutType,sizeof(int)*numOutRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMalloc((void***)&d_pDataIn,sizeof(void*)*numInRaster));
	  checkCudaErrors(cudaMalloc((void***)&d_pDataOut,sizeof(void*)*numOutRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataIn,pDataIn,sizeof(void*)*numInRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMemcpy(d_pDataOut,pDataOut,sizeof(void*)*numOutRaster,cudaMemcpyHostToDevice));
	  //checkCudaErrors(cudaMemcpy(d_nbrcoord,cucoords, sizeof(int)*nbrsize * 2, cudaMemcpyHostToDevice));
	  //checkCudaErrors( cudaMemcpy(d_weights, weights, sizeof(double)*nbrsize, cudaMemcpyHostToDevice));
	 // dim3 block(16,16);
	//  dim3 grid(nwidth% 16 == 0 ? nwidth /16 : nwidth / 16 + 1, nheight % 16 == 0 ? nheight /16 : nheight /16 + 1);
	  int block=256;
	  int gird=_smCount*24;
	  G_ZonalMutiOperator<OperType> <<<gird,block>>> (d_pDataIn,d_pDataInType, d_pDataOut, d_pDataOutType, nwidth, nheight, cellWidth, cellHeight,d_paramInfo, OperType());
	  cudaDeviceSynchronize();
	  
	  for(int i=0;i<numOutRaster;i++)
	  {

		  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		  checkCudaErrors(cudaMemcpy(outdata[i],pDataOut[i],nwidth*nheight*pRrmSpc->info()->dataSize(),cudaMemcpyDeviceToHost));
		  pRrmSpc->brupdateCell(br,outdata[i]);
	  }
	  for(int i=0;i<numOutRaster;i++)
	  {
		  delete []outdata[i];
	  }
	  cudaFree(d_pDataInType);
	  cudaFree(d_pDataOutType);
	  checkCudaErrors(cudaFree(d_paramInfo));
//	  cudaFree(d_weights);
//	  cudaFree(d_nbrcoord);
	  return mcRPL::EVAL_SUCCEEDED;
	 
  }

  template<class OperType>
  mcRPL::EvaluateReturn mcRPL::Transition::cuLocalOperator(const mcRPL::CoordBR &br)
  {
	 // mcRPL::Cellspace *pRrmSpc=getCellspaceByLyrName(getInLyrNames()[0]);
	 // DataInType *d_input;
	 // DataOutType *d_output;
	 // DataInType *d_inputnodata;
	 // DataOutType *d_outputnodata;
	 // double* d_paramInfo;
	 // int nwidth=pRrmSpc->info()->dims().nCols();
	 // int nheight=pRrmSpc->info()->dims().nRows();
	 // int numInRaster=_vInLyrNames.size();
	 // int numOutRaster=_vOutLyrNames.size();
	 //  DataOutType *output=new DataOutType[nwidth*nheight*numOutRaster];
	 // vector<DataInType>nodataIn;
	 //  vector<DataOutType>nodataOut;
	 // checkCudaErrors(cudaMalloc(&d_input,sizeof(DataInType)*nwidth*nheight*numInRaster));
	 // checkCudaErrors(cudaMalloc(&d_output,sizeof(DataOutType)*nwidth*nheight*numOutRaster));
	 // checkCudaErrors(cudaMalloc(&d_inputnodata,sizeof(DataInType)*numInRaster));
	 // checkCudaErrors(cudaMalloc(&d_outputnodata,sizeof(DataOutType)*numOutRaster));
	 // if(_vparamInfo.size()!=0)
	 // {
		//  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)*_vparamInfo.size()));
		//  checkCudaErrors(cudaMemcpy(d_paramInfo,&_vparamInfo[0],sizeof(double)*_vparamInfo.size(),cudaMemcpyHostToDevice));
	 // }
	 // else
	 // {
		//  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)));
	 // }
	 // for(int i=0;i<numInRaster;i++)
	 // {
		//  pRrmSpc=getCellspaceByLyrName(getInLyrNames()[i]);
		//  checkCudaErrors(cudaMemcpy(d_input+i*nwidth*nheight,pRrmSpc->getData<DataInType>(),sizeof(DataInType)*nwidth*nheight,cudaMemcpyHostToDevice));
		//  nodataIn.push_back(pRrmSpc->info()->getNoDataValAs<DataInType>());
	 // }
	 // for(int i=0;i<numOutRaster;i++)
	 // {
		//  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		//  nodataOut.push_back(pRrmSpc->info()->getNoDataValAs<DataOutType>());
	 // }
	 // checkCudaErrors(cudaMemcpy(d_inputnodata,&nodataIn[0],sizeof(DataInType)*numInRaster,cudaMemcpyHostToDevice));
	 // checkCudaErrors(cudaMemcpy(d_outputnodata,&nodataOut[0],sizeof(DataInType)*numOutRaster,cudaMemcpyHostToDevice));
	 // dim3 block(16,16);
  //   dim3 grid(nwidth% 16 == 0 ? nwidth /16 : nwidth / 16 + 1, nheight % 16 == 0 ? nheight /16 : nheight /16 + 1);
	 //// dim3 grid = mcRPL::CuEnvControl::getGrid(nwidth, nheight);
	 // G_LocalOperMultiLayersPaWithNoData<DataInType, DataOutType, OperType> << <grid, block >> >(d_input, d_output, nwidth, nheight, d_inputnodata, d_outputnodata, numInRaster,numOutRaster,d_paramInfo,_vparamInfo.size(), OperType());
	 // checkCudaErrors(cudaMemcpy(output,d_output,sizeof(DataOutType)*nwidth*nheight*numOutRaster,cudaMemcpyDeviceToHost));
	 // for(int i=0;i<numOutRaster;i++)
	 // {
		//  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		//  for(long iRow = br.minIRow(); iRow <= br.maxIRow(); iRow++) 
		//  {
		//	  for(long iCol = br.minICol(); iCol <= br.maxICol(); iCol++) 
		//	  {
		//		  /* done = evaluate(mcRPL::CellCoord(iRow, iCol));
		//		  if(done == mcRPL::EVAL_FAILED ||
		//		  done == mcRPL::EVAL_TERMINATED) {
		//		  return done;
		//		  }*/

		//		  pRrmSpc->updateCellAs<DataOutType>(mcRPL::CellCoord(iRow, iCol),output[iRow*nwidth+iCol+nwidth*nheight*i], true);
		//	  }
		//  }
	 // }
	 // checkCudaErrors(cudaFree(d_input));
	 // checkCudaErrors(cudaFree(d_inputnodata));
	 // checkCudaErrors(cudaFree(d_outputnodata));
	 // checkCudaErrors(cudaFree(d_output));
	 // checkCudaErrors(cudaFree(d_paramInfo));
	 // delete []output;
	 // return mcRPL::EVAL_SUCCEEDED;
	// int nbrsize =_pNbrhd->size();
	//  double *d_weights;
	//  vector<int>cuNbrCoords=_pNbrhd->cuCoords();
	//  vector<double>cuNbrWeights=_pNbrhd->cuWeights();
	  int brminIRow=br.minIRow();
	  int brmaxIRow=br.maxIRow();
	  int brminICol=br.minICol();
	  int brmaxICol=br.maxICol();
	  mcRPL::Cellspace *pRrmSpc=getCellspaceByLyrName(getPrimeLyrName());
	  int nwidth=pRrmSpc->info()->dims().nCols();
	  int nheight=pRrmSpc->info()->dims().nRows();
	  if(brminIRow!=0||brminICol!=0||brmaxIRow!=(nheight-1)|| brmaxICol!=(nwidth-1))
	  {
		  cout<<"you has defined a nrb,please use FocalOperator";
		  return mcRPL::EVAL_FAILED;
	  }
	 // int *cucoords = &cuNbrCoords[0];
	//  double *weights=&cuNbrWeights[0];
	  double cellWidth =fabs(pRrmSpc->info()->georeference()->cellSize().x());
	  double cellHeight =fabs(pRrmSpc->info()->georeference()->cellSize().y());
	  int numInRaster=_vInLyrNames.size();
	  int numOutRaster=_vOutLyrNames.size();
	  void **pDataIn=new void *[numInRaster];
	  void **pDataOut=new void *[numOutRaster];
	  int *pDataInType=new int[numInRaster];
	  int *pDataOutType=new int[numOutRaster];
	  void **d_pDataIn;
	  void **d_pDataOut;
	  int *d_pDataInType;
	  int *d_pDataOutType;
	//  int *d_nbrcoord;
	  void **outdata=new void *[numOutRaster];
	  double* d_paramInfo;
	   if(_vparamInfo.size()!=0)
	  {
		  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)*_vparamInfo.size()));
		  checkCudaErrors(cudaMemcpy(d_paramInfo,&_vparamInfo[0],sizeof(double)*_vparamInfo.size(),cudaMemcpyHostToDevice));
	  }
	  else
	  {
		  checkCudaErrors(cudaMalloc(&d_paramInfo,sizeof(double)));
	  }
	 // checkCudaErrors(cudaMalloc(&d_nbrcoord, sizeof(int)*nbrsize * 2));
	  //checkCudaErrors(cudaMalloc(&d_weights, sizeof(double)*nbrsize));
	  for(int i=0;i<numInRaster;i++)
	  {
		  pRrmSpc=getCellspaceByLyrName(getInLyrNames()[i]);
		   pDataIn[i]=pRrmSpc->getGPUData();
		   pDataInType[i]=pRrmSpc->info()->cudaTypeCopy();
	//	   cout<< pDataInType[i]<<endl;
	  }
	  for(int i=0;i<numOutRaster;i++)
	  {
		  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		  outdata[i]=new char[nwidth*nheight*pRrmSpc->info()->dataSize()];
		  pDataOut[i]=pRrmSpc->getGPUData();
		  pDataOutType[i]=pRrmSpc->info()->cudaTypeCopy();
	//	  cout<< pDataOutType[i]<<endl;
	  }
	  checkCudaErrors(cudaMalloc(&d_pDataInType,sizeof(int)*numInRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataInType,pDataInType,sizeof(int)*numInRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMalloc(&d_pDataOutType,sizeof(int)*numOutRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataOutType,pDataOutType,sizeof(int)*numOutRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMalloc((void***)&d_pDataIn,sizeof(void*)*numInRaster));
	  checkCudaErrors(cudaMalloc((void***)&d_pDataOut,sizeof(void*)*numOutRaster));
	  checkCudaErrors(cudaMemcpy(d_pDataIn,pDataIn,sizeof(void*)*numInRaster,cudaMemcpyHostToDevice));
	  checkCudaErrors(cudaMemcpy(d_pDataOut,pDataOut,sizeof(void*)*numOutRaster,cudaMemcpyHostToDevice));
	  //checkCudaErrors(cudaMemcpy(d_nbrcoord,cucoords, sizeof(int)*nbrsize * 2, cudaMemcpyHostToDevice));
	  //checkCudaErrors( cudaMemcpy(d_weights, weights, sizeof(double)*nbrsize, cudaMemcpyHostToDevice));
	 // dim3 block(16,16);
	 // dim3 grid(nwidth% 16 == 0 ? nwidth /16 : nwidth / 16 + 1, nheight % 16 == 0 ? nheight /16 : nheight /16 + 1);
	   int block=256;
	  int grid=_smCount*24;
	  G_LocalMutiOperator<OperType> <<< grid,block>>> (d_pDataIn,d_pDataInType, d_pDataOut, d_pDataOutType, nwidth, nheight, cellWidth, cellHeight,d_paramInfo, OperType());
	  cudaDeviceSynchronize();
	  
	  for(int i=0;i<numOutRaster;i++)
	  {

		  pRrmSpc=getCellspaceByLyrName(getOutLyrNames()[i]);
		  checkCudaErrors(cudaMemcpy(outdata[i],pDataOut[i],nwidth*nheight*pRrmSpc->info()->dataSize(),cudaMemcpyDeviceToHost));
		  pRrmSpc->brupdateCell(br,outdata[i]);
	  }
	  for(int i=0;i<numOutRaster;i++)
	  {
		  delete []outdata[i];
	  }
	  cudaFree(d_pDataInType);
	  cudaFree(d_pDataOutType);
	  checkCudaErrors(cudaFree(d_paramInfo));
//	  cudaFree(d_weights);
//	  cudaFree(d_nbrcoord);
	  return mcRPL::EVAL_SUCCEEDED;
  }
  template<class DataInType, class DataOutType, class OperType>
  mcRPL::EvaluateReturn mcRPL::Transition::cuFocalOperator(const mcRPL::CoordBR &br)
  {

	  mcRPL::Cellspace *pPrmSpc = getCellspaceByLyrName(getInLyrNames()[0]);
	  vector<mcRPL::WeightedCellCoord>coords =_pNbrhd->getInnerNbr(); 
	  int nbrsize =_pNbrhd->size();
	  vector<int>cuNbrCoords;
	  vector<double>cuNbrWeights;
	  for(int iNbr = 0; iNbr < nbrsize; iNbr++)
	  {
		  /* if(coords[iNbr].iRow() == 0 && coords[iNbr].iCol() == 0 ) 
		  {
		  continue;
		  }*/
		  cuNbrCoords.push_back(coords[iNbr].iRow());
		  cuNbrCoords.push_back(coords[iNbr].iCol());
		  cuNbrWeights.push_back(coords[iNbr].weight());
	  }
//	  cuNbrCoords.push_back(0);
	//  cuNbrCoords.push_back(0);
//	  cuNbrWeights.push_back(1);
	  int *cucoords = &cuNbrCoords[0];
	  double* weights = &cuNbrWeights[0];
	  double cellWidth =fabs(pPrmSpc->info()->georeference()->cellSize().x());
	  double cellHeight =fabs(pPrmSpc->info()->georeference()->cellSize().y());
	  int width=pPrmSpc->info()->dims().nCols();
	  int height=pPrmSpc->info()->dims().nRows();
	  mcRPL::Cellspace *pIptSpc = getCellspaceByLyrName(getInLyrNames()[0]);
	  DataInType *input = pIptSpc->getData<DataInType>();
	  DataInType *d_input;
	  DataOutType *d_output;
	  double *d_weights;
	  int *d_nbrcoord;
	  DataOutType *output=new DataOutType[width*height];
	  mcRPL::checkCudaErrors(cudaMalloc(&d_input, sizeof(DataInType)*width*height));
	 mcRPL:: checkCudaErrors(cudaMalloc(&d_output, sizeof(DataOutType)*width*height));
	  mcRPL:: checkCudaErrors(cudaMalloc(&d_nbrcoord, sizeof(int)*nbrsize * 2));
	 mcRPL:: checkCudaErrors(cudaMalloc(&d_weights, sizeof(double)*nbrsize));

	  mcRPL::checkCudaErrors( cudaMemcpy(d_input, input, sizeof(DataInType)*width*height, cudaMemcpyHostToDevice));
	  mcRPL::checkCudaErrors(cudaMemcpy(d_nbrcoord,cucoords, sizeof(int)*nbrsize * 2, cudaMemcpyHostToDevice));
	  mcRPL::checkCudaErrors( cudaMemcpy(d_weights, weights, sizeof(double)*nbrsize, cudaMemcpyHostToDevice));
	  mcRPL::Cellspace *_pSlpCellspc = getCellspaceByLyrName(_vOutLyrNames[0]);
	  DataOutType _slpNoData = _pSlpCellspc->info()->getNoDataValAs<DataOutType>();
	  dim3 block(16,16);
	  dim3 grid(width% 16 == 0 ? width /16 : width / 16 + 1, height % 16 == 0 ? height /16 : height /16 + 1);
	  //dim3 block =mcRPL::CuEnvControl::getBlock2D();
	  //dim3 grid = mcRPL::CuEnvControl::getGrid(width, height);
	  G_FocalOperator<DataInType, DataOutType, double,OperType> <<< grid,block>>> (d_input, d_output, width, height,d_nbrcoord,d_weights,   nbrsize,_slpNoData, cellWidth, cellHeight, OperType());

	  mcRPL:: checkCudaErrors(cudaMemcpy(output, d_output, sizeof(DataOutType)*width*height, cudaMemcpyDeviceToHost));


	  for(long iRow = br.minIRow(); iRow <= br.maxIRow(); iRow++) 
	  {
		  for(long iCol = br.minICol(); iCol <= br.maxICol(); iCol++) 
		  {
			  /* done = evaluate(mcRPL::CellCoord(iRow, iCol));
			  if(done == mcRPL::EVAL_FAILED ||
			  done == mcRPL::EVAL_TERMINATED) {
			  return done;
			  }*/
			  _pSlpCellspc->updateCellAs<DataOutType>(mcRPL::CellCoord(iRow, iCol),output[iRow*width+iCol], true);
		  }
	  }
	  delete []output;
	  cudaFree(d_input);
	  cudaFree(d_output);
	  cudaFree(d_nbrcoord);
	  return mcRPL::EVAL_SUCCEEDED;
  }
};

#endif
