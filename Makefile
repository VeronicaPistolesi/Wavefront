.PHONY: all clean

# Nome dei compilatori
GPP = g++
MPICXX = mpicxx

# Flag di compilazione
GPP_FLAGS = -O2 -pthread -I .
MPICXX_FLAGS = -O2 -I . -fopenmp

# Directory di output
BUILD_DIR = ./build

# Lista dei file sorgenti
GPP_SOURCES = sequential.cpp sequential_t.cpp sequential_t1m.cpp fastflow_pfor.cpp fastflow_pforgrain.cpp 
MPICXX_SOURCES = mpi.cpp mpi_w0_ll.cpp mpi_w0_async_ll.cpp mpi_w0.cpp mpi_w0_async.cpp 

# Converte i nomi dei file sorgenti in nomi degli eseguibili nella cartella build
GPP_TARGETS = $(GPP_SOURCES:%.cpp=$(BUILD_DIR)/%)
MPICXX_TARGETS = $(MPICXX_SOURCES:%.cpp=$(BUILD_DIR)/%)

# Regola principale
all: $(GPP_TARGETS) $(MPICXX_TARGETS)

# Regola per creare la directory di build
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Regola per compilare i file con g++
$(GPP_TARGETS): $(BUILD_DIR)/%: %.cpp | $(BUILD_DIR)
	$(GPP) $(GPP_FLAGS) $< -o $@

# Regola per compilare i file con mpicxx
$(MPICXX_TARGETS): $(BUILD_DIR)/%: %.cpp | $(BUILD_DIR)
	$(MPICXX) $(MPICXX_FLAGS) $< -o $@

# Pulizia dei file compilati
clean:
	rm -rf $(BUILD_DIR)
