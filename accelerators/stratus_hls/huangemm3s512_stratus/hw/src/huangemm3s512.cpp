// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "huangemm3s512.hpp"
#include "huangemm3s512_directives.hpp"

// Functions

#include "huangemm3s512_functions.hpp"

// Processes

void huangemm3s512::load_input()
{

    // Reset
    {
        HLS_PROTO("load-reset");

        this->reset_load_input();

        // explicit PLM ports reset if any

        // User-defined reset code

        wait();
    }

    // Config
    /* <<--params-->> */
    int32_t rows;
    int32_t cols;
    int32_t loaded_cols;
    {
        HLS_PROTO("load-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        rows = config.rows;
        cols = config.cols;
        loaded_cols = config.loaded_cols;
    }
    // ESP_REPORT_INFO("CPP -- load -- config done");

    // Load
    {
        HLS_PROTO("load-dma");
        wait();

        


        // bool ping = true;
        bool pingA = true;
        bool pingB = true;
            
        for (int lcn = 0; lcn < loaded_cols; lcn++) {

            // ******* B part: here we load matrix B ******
            int b_length = cols;
            int tb1 = rows*loaded_cols;
            int tb2 = lcn*cols;
            wait();
            int load_B_offset = round_up(tb1+tb2, DMA_WORD_PER_BEAT);
            // make cols becomes continuous
            for (int rem = b_length; rem > 0; rem -= PLM_IN_WORD) {
                // I am using 4*4 matrix to test, so PLM_IN_WORD is 16
                uint32_t len = rem > PLM_IN_WORD ? PLM_IN_WORD : rem;
                dma_info_t dma_info_B(load_B_offset / DMA_WORD_PER_BEAT, len / DMA_WORD_PER_BEAT, DMA_SIZE);
                load_B_offset += len;
                this->dma_read_ctrl.put(dma_info_B);

                for (uint16_t i = 0; i < len; i += DMA_WORD_PER_BEAT) {
                    HLS_BREAK_DEP(B_ping);
                    HLS_BREAK_DEP(B_pong);
                    sc_dt::sc_bv<DMA_WIDTH> dataBv;
                    dataBv = this->dma_read_chnl.get();
                    wait();

                    // Write to PLM (all DMA_WORD_PER_BEAT words in one cycle)
                    for (uint16_t k = 0; k < DMA_WORD_PER_BEAT; k++) {
                        // ESP_REPORT_INFO("getting B[%d]\n", i*DMA_WORD_PER_BEAT + k);
                        HLS_UNROLL_SIMPLE;
                        if (pingB) {
                            B_ping[i + k] = dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
                        } else {
                            B_pong[i + k] = dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
                        }
                        
                    }
                }
            }
            wait();

            for(int cr = 0; cr < rows; cr+=2) {
                // *********===== O part: here we also load the previous result O ==== *********
                // ******* we need to load previous O result *******
            

                uint32_t load_O_length = round_up(2*cols, DMA_WORD_PER_BEAT);
                uint32_t tmp1 = loaded_cols*(cols+rows);
                uint32_t tmp2 = cr*cols;
                wait();
                uint32_t load_O_offset = round_up(tmp1 + tmp2, DMA_WORD_PER_BEAT);
                dma_info_t dma_info_O(load_O_offset / DMA_WORD_PER_BEAT, load_O_length / DMA_WORD_PER_BEAT, DMA_SIZE);
                this->dma_read_ctrl.put(dma_info_O);
                // std::cout << "load_O_offset: " << load_O_offset << std::endl;
                
                for (uint16_t i = 0; i < load_O_length; i += DMA_WORD_PER_BEAT)
                {
                    HLS_BREAK_DEP(O_ping);
                    HLS_BREAK_DEP(O_pong);
                    sc_dt::sc_bv<DMA_WIDTH> dataBv;
                    dataBv = this->dma_read_chnl.get();
                    // Read from PLM
                    wait();
                    for (uint16_t k = 0; k < DMA_WORD_PER_BEAT; k++)
                    { 
                        HLS_UNROLL_SIMPLE;
                        if (pingA) {
                            O_ping[i + k] = (lcn == 0)? 0:dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
                        } else {
                            O_pong[i + k] = (lcn == 0)? 0:dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
                        }
                    }
                }

                // dump to check the value
                // std::cout << "===============load: dump o_ping ==============\n";
                // for (uint16_t i = 0; i < load_O_length; i++)
                // {
                //     std::cout << "O_ping[" << i << "] = " << int2fp<FPDATA, WORD_SIZE>(O_ping[i]) << std::endl;
                //     // std::cout << "O_pong[" << i << "] = " << O_pong[i] << std::endl;
                // }

                // std::cout << "===============load: dump o_pong ==============\n";
                // for (uint16_t i = 0; i < load_O_length; i++)
                // {
                //     // std::cout << "O_ping[" << i << "] = " << O_ping[i] << std::endl;
                //     std::cout << "O_pong[" << i << "] = " << int2fp<FPDATA, WORD_SIZE>(O_pong[i]) << std::endl;
                // }
                // std::cout << "========================\n";

                // ******* A part: here we load matrix A ******
                // std::cout << "load: lcn = " << lcn << std::endl;
                // std::cout << "load: row = " << cr << std::endl;
                // std::cout << "load: pingA = " << pingA << std::endl;
                int a_length = 2;
                uint32_t load_A_offset = lcn*rows + cr;
                // make rows becomes continuous
                for (int rem = a_length; rem > 0; rem -= PLM_IN_WORD) {
                    // I am using 4*4 matrix to test, so PLM_IN_WORD is 16
                    // std::cout << "Hello?" << std::endl;
                    uint32_t len = rem > PLM_IN_WORD ? PLM_IN_WORD : rem;
                    dma_info_t dma_info_A(load_A_offset / DMA_WORD_PER_BEAT, len / DMA_WORD_PER_BEAT, DMA_SIZE);
                    // std::cout << "Hello1?" << std::endl;
                    load_A_offset += len;
                    this->dma_read_ctrl.put(dma_info_A);
                    // std::cout << "Hello2?" << std::endl;
                    for (uint16_t i = 0; i < len; i += DMA_WORD_PER_BEAT) {
                        HLS_BREAK_DEP(A_ping);
                        HLS_BREAK_DEP(A_pong);

                        sc_dt::sc_bv<DMA_WIDTH> dataBv;
                        // std::cout << "Hello3?" << std::endl;
                        dataBv = this->dma_read_chnl.get();
                        // std::cout << "Hello4?" << std::endl;
                        wait();

                        // Write to PLM (all DMA_WORD_PER_BEAT words in one cycle)
                        for (uint16_t k = 0; k < DMA_WORD_PER_BEAT; k++) {
                            HLS_UNROLL_SIMPLE;
                            if (pingA) {
                                A_ping[i + k] = dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
                                // std::cout << "load: loading A_ping[" << i+k << "] = " << A_ping[i + k] << endl;
                            } else {
                                A_pong[i + k] = dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
                                // std::cout << "load: loading A_pong[" << i+k << "] = " << A_pong[i + k] << endl;
                            }
                            
                        }
                    }
                }
                wait();
                
                // std::cerr << "load: before load compute handshake. lcn = " << lcn << ", cr = " << cr << std::endl;
                this->load_compute_handshake();
                // std::cerr << "load: load compute handshake, move to another part (prev lcn = " << lcn << ", cr = " << cr << std::endl;
                pingA = !pingA;
            }
            pingB = !pingB;
        }

        
        

        
        // ESP_REPORT_INFO("end load compute handshake");
    }



    // Conclude
    {
        this->process_done();
    }
}



void huangemm3s512::store_output()
{
    // Reset
    {
        HLS_PROTO("store-reset");

        this->reset_store_output();

        // explicit PLM ports reset if any

        // User-defined reset code

        wait();
    }

    // Config
    /* <<--params-->> */
    int32_t rows;
    int32_t cols;
    int32_t loaded_cols;
    {
        HLS_PROTO("store-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        rows = config.rows;
        cols = config.cols;
        loaded_cols = config.loaded_cols;
    }

    // Store
    {

        HLS_PROTO("store-dma");
        wait();
        // bool ping = true;


        bool pingA = true;
        bool pingB = true;

        // uint32_t store_offset = round_up(loaded_cols*cols, DMA_WORD_PER_BEAT) * rows;
        // uint32_t offset = store_offset;

        // TPH: I don't really understand when the offset does not start from zero
        // The offset is sizeA + sizeB

        
        // operating mode 3: load a rows element and a column into plm, then switch to another row element and another column, and so on
        
            // TODO: here I will need to handle the case when col is odd number
            

        for (int lcn = 0; lcn < loaded_cols; lcn++) {
            for (int cr = 0; cr < rows; cr+=2) {

                uint32_t length = round_up(2*cols, DMA_WORD_PER_BEAT);
                uint32_t store_O_offset = round_up(loaded_cols*(cols+rows)+cr*cols, DMA_WORD_PER_BEAT);

                // std::cout << "store: pingB = " << pingB << std::endl;
                this->store_compute_handshake();
                // std::cout << "store: store compute handshake" << std::endl;
                // std::cout << "store: ===== loaded cols: ====" << lcn << std::endl;
                // std::cout << "compute: rows: " << cr << std::endl;
                // std::cout << "store: pingA = " << pingA << std::endl;
                // std::cout << "store_O_offset: " << store_O_offset << std::endl;

                dma_info_t dma_info(store_O_offset / DMA_WORD_PER_BEAT, length / DMA_WORD_PER_BEAT, DMA_SIZE);
                this->dma_write_ctrl.put(dma_info);
                
                for (uint16_t i = 0; i < length; i += DMA_WORD_PER_BEAT)
                {
                    sc_dt::sc_bv<DMA_WIDTH> dataBv;

                    // Read from PLM
                    wait();
                    for (uint16_t k = 0; k < DMA_WORD_PER_BEAT; k++)
                    {
                        HLS_UNROLL_SIMPLE;
                        if (pingA) {
                            dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH) = O_ping[i + k];
                        } else {
                            dataBv.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH) = O_pong[i + k];
                        }
                    }
                    // ESP_REPORT_INFO("before put");
                    this->dma_write_chnl.put(dataBv);
                    // ESP_REPORT_INFO("after put");
                }


                pingA = !pingA;
                store_O_offset += length;
            }
            wait();
            pingB = !pingB;
        }

            
        
        

    }

    // for(int i = 0; i < 16; i++) {
    //     ESP_REPORT_INFO("O_ping[%d] = %d\n", i, O_ping[i].to_int64());
    // }
    // Conclude
    {
        this->accelerator_done();
        this->process_done();
    }
}


void huangemm3s512::compute_kernel()
{
    // Reset
    {
        HLS_PROTO("compute-reset");

        this->reset_compute_kernel();

        // explicit PLM ports reset if any

        // User-defined reset code

        wait();
    }

    // Config
    /* <<--params-->> */
    int32_t rows;
    int32_t cols;
    int32_t loaded_cols;
    {
        HLS_PROTO("compute-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        rows = config.rows;
        cols = config.cols;
        loaded_cols = config.loaded_cols;
    }


    // Compute
    {
        HLS_PROTO("compute-real");
        
        // ESP_REPORT_INFO("end compute load handshake");

        // we need to plan how much to load based on the current configuration
        // we need to block the case that (1 + cols) > plm size
        
        // bool ping = true;

        bool pingA = true;
        bool pingB = true;
        
        

        
        // operating mode 3: load a rows element and a column into plm, then switch to another row element and another column, and so on
       
        for (int i = 0; i < PLM_OUT_WORD; i++) {
            O_ping[i] = 0;
            O_pong[i] = 0;
            wait();
        }
        
        for (int i = 0; i < loaded_cols; i++){
            // std::cout << "compute: ===== loaded cols: ==== " << i << std::endl;
            for (int j = 0; j < rows; j+=2) {
                this -> compute_load_handshake();
                // std::cout << "compute: compute load handshake" << std::endl;
                for (int t = 0; t < 2; t++) {
                    
                    
                    // std::cout << "compute: rows: " << j+t << std::endl;
                    // std::cout << "compute: pingA = " << pingA << std::endl;
                    // std::cout << "compute: pingB = " << pingB << std::endl;
                    for (int k = 0; k < cols; k++) {
                        // wait();
                        wait();
                        FPDATA tmp1;
                        if (pingA) {
                            tmp1 = int2fp<FPDATA, WORD_SIZE>(O_ping[t*cols + k]);
                        } else {
                            tmp1 = int2fp<FPDATA, WORD_SIZE>(O_pong[t*cols + k]);
                        }
                        
                        wait();
                        FPDATA tmp2;
                        FPDATA tmp3;
                        if (pingA) {
                            tmp2 = int2fp<FPDATA, WORD_SIZE>(A_ping[t]);
                        } else {
                            tmp2 = int2fp<FPDATA, WORD_SIZE>(A_pong[t]);
                        }
                        
                        wait();
                        if (pingB) {
                            tmp3 = int2fp<FPDATA, WORD_SIZE>(B_ping[k]);
                        } else {
                            tmp3 = int2fp<FPDATA, WORD_SIZE>(B_pong[k]);
                        }
                        
                        wait();
                        FPDATA tmp4 = tmp1 + tmp2 * tmp3;
                        // std::cout << "compute: (rows, cols) = (" << j+t << ", " << k << ")" << std::endl;
                        // std::cout << "compute: O: " << tmp1 << std::endl;
                        // std::cout << "compute: A: " << tmp2 << std::endl;
                        // std::cout << "compute: B: " << tmp3 << std::endl;
                        // std::cout << "compute: new O: " << tmp4 << std::endl;
                        wait();
                        if (pingA) {
                            O_ping[t*cols + k] = fp2int<FPDATA, WORD_SIZE>(tmp4);
                        } else {
                            O_pong[t*cols + k] = fp2int<FPDATA, WORD_SIZE>(tmp4);
                        }
                        
                        wait();

                    }
                    wait();
                }
                
                wait();
                pingA = !pingA;
                this->compute_store_handshake();
            }
            wait();
            pingB = !pingB;
        }
            
        
        
        // ESP_REPORT_INFO("end compute store handshake");
    }

    // Conclude
    {
        this->process_done();
    }

    
}
