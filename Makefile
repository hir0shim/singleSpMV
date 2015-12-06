PREFIX = test
SOURCE_DIR = src
BINARY_DIR = bin
OBJECT_DIR = obj
LIBRARY_DIR = lib
INCLUDE_DIR = include
ASM_DIR=asm


# -- Params list ---
# COMMON  : [INDEX_32,INDEX_64]
# OPT_SS  : [SIMPLE,OPTIMIZED] PADDING COEF_SEGMENT_WIDTH MEASURE_STEP_TIME 
# OPT_CSS : [SIMPLE,OPTIMIZED] PADDING N_BLOCK 

#OPTION = -DOPT_SS -DSIMPLE -DCOEF_SEGMENT_WIDTH=64
OPTION = -DOPT_CSS -DOPTIMIZED -DN_BLOCK=4

# CPU,MIC,GPU,VERIFY,OPT_[CRS,MKL,CUSPARSE,JDS,ELL,COO,SS,CSS],ALIGNMENT,INDEX_[32,64]
MIC_OPTION = $(OPTION) -DMIC -DVERIFY -DINDEX_32 -DALIGNMENT=64 
CPU_OPTION = $(OPTION) -DCPU -DVERIFY -DINDEX_32 -DALIGNMENT=32 -xHOST
GPU_OPTION = $(OPTION) -DGPU -DVERIFY -DOPT_CUSPARSE -DINDEX_32 -DALIGNMENT=32

CXX = icpc
LDFLAGS = -L$(LIBRARY_DIR) -L$(OBJECT_DIR) 
CXXFLAGS = -std=c++11 -ipo -Wall -restrict -O3 -fopenmp -fno-inline -g

vpath %.cpp $(SOURCE_DIR)
spmv_sources = main.cpp util.cpp opt.cpp 

spmv_objects_cpu = $(addprefix $(OBJECT_DIR)/, $(spmv_sources:.cpp=.$(PREFIX).o.cpu))
spmv_objects_mic = $(addprefix $(OBJECT_DIR)/, $(spmv_sources:.cpp=.$(PREFIX).o.mic))
spmv_objects_gpu = $(addprefix $(OBJECT_DIR)/, $(spmv_sources:.cpp=.$(PREFIX).o.gpu))


SPMV_CPU=$(BINARY_DIR)/$(PREFIX)-spmv.cpu
SPMV_MIC=$(BINARY_DIR)/$(PREFIX)-spmv.mic
# SPMV_GPU=$(BINARY_DIR)/$(PREFIX)-spmv.gpu
TARGETS=$(SPMV_CPU) $(SPMV_MIC) $(SPMV_GPU)

all: $(TARGETS)
cpu: $(SPMV_CPU)
mic: $(SPMV_MIC)
gpu: $(SPMV_GPU)


########################################
# SPMV CPU 
########################################
$(OBJECT_DIR)/%.o.cpu : CXXFLAGS += $(CPU_OPTION)
$(OBJECT_DIR)/%.$(PREFIX).o.cpu : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(SPMV_CPU) : CXXFLAGS += -mkl 
$(SPMV_CPU) : $(spmv_objects_cpu) 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

########################################
# SPMV MIC
########################################
$(OBJECT_DIR)/%.o.mic : CXXFLAGS += -mmic $(MIC_OPTION) 
$(OBJECT_DIR)/%.$(PREFIX).o.mic : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(SPMV_MIC) : CXXFLAGS += -mmic -mkl 
$(SPMV_MIC) : $(spmv_objects_mic)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

########################################
# SPMV GPU 
########################################
$(OBJECT_DIR)/%.o.gpu : CXXFLAGS += -xHOST -I/usr/local/cuda-6.5/include $(GPU_OPTION)
$(OBJECT_DIR)/%.$(PREFIX).o.gpu : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SPMV_GPU) : CXXFLAGS += -L/usr/local/cuda-6.5/lib64 -lcusparse -lcudart 
$(SPMV_GPU) : $(spmv_objects_gpu) 
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

########################################
# Others
########################################

check :
	@echo $(objects)

clean : 
	rm -f $(spmv_objects_cpu) 
	rm -f $(spmv_objects_mic) 
	rm -f $(spmv_objects_gpu) 
	rm -f $(partition_objects)
	rm -f $(SPMV_CPU)
	rm -f $(SPMV_MIC)
	rm -f $(SPMV_GPU)
	rm -f $(PARTITION)

.PHONY : all clean check
