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
GPP_SOURCES = sequential.cpp sequential_t.cpp fastflow_pfor.cpp fastflow_pfor_t.cpp fastflow_pforgrain.cpp fastflow_pforgrain_t.cpp
MPICXX_SOURCES = mpi.cpp mpi_t.cpp mpi_new.cpp mpi_new_t.cpp

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
