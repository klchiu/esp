# Copyright (c) 2011-2021 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

############################################################
# Design Parameters
############################################################

#
# Source the common configurations
#
source ../../../common/hls/project.tcl


#
# Set the private memory library
#
use_hls_lib "./memlib"


#
# Local synthesis attributes
#
if {$TECH eq "virtex7"} {
    # Library is in ns, but simulation uses ps!
    set CLOCK_PERIOD 10.0
    set SIM_CLOCK_PERIOD 10000.0
    set_attr default_input_delay      0.1
}
if {$TECH eq "zynq7000"} {
    # Library is in ns, but simulation uses ps!
    set CLOCK_PERIOD 10.0
    set SIM_CLOCK_PERIOD 10000.0
    set_attr default_input_delay      0.1
}
if {$TECH eq "virtexu"} {
    # Library is in ns, but simulation uses ps!
    set CLOCK_PERIOD 8
    set SIM_CLOCK_PERIOD 8000.0
    set_attr default_input_delay      0.1
}
if {$TECH eq "virtexup"} {
    # Library is in ns, but simulation uses ps!
    set CLOCK_PERIOD 10
    set SIM_CLOCK_PERIOD 10000.0
    set_attr default_input_delay      0.1
}
if {$TECH eq "cmos32soi"} {
    set CLOCK_PERIOD 1000.0
    set SIM_CLOCK_PERIOD 1000.0
    set_attr default_input_delay      100.0
}
if {$TECH eq "gf12"} {
    set CLOCK_PERIOD 750.0
    set SIM_CLOCK_PERIOD 2000.0
    set_attr default_input_delay      100.0
}
set_attr clock_period $CLOCK_PERIOD

#
# Common options for all configurations
#
#set COMMON_HLS_FLAGS \
#    "-DFLOAT_POINT --clock_period=$CLOCK_PERIOD"
#set COMMON_CFG_FLAGS \
#    "-DFLOAT_POINT --DCLOCK_PERIOD=$SIM_CLOCK_PERIOD"


#
# System level modules to be synthesized
#
define_hls_module tf_mult3 ../src/tf_mult3.cpp


#
# Testbench or system level modules
#
define_system_module tb ../tb/system.cpp ../tb/sc_main.cpp


set_attr split_multiply 32
#set_attr split_multiply 16
#set_attr split_add 32
#set_attr dpopt_auto all
#set_attr dpopt_with_enable on


######################################################################
# HLS and Simulation configurations
######################################################################
set DEFAULT_ARGV ""

foreach dma_width [list 64] {

    set ext DMA$dma_width

    define_io_config * IOCFG_$ext -DDMA_WIDTH=$dma_width

    define_system_config tb TESTBENCH_$ext -io_config IOCFG_$ext

    define_sim_config "BEHAV_$ext" "tf_mult3 BEH" "tb TESTBENCH_$ext" \
        -io_config IOCFG_$ext -argv $DEFAULT_ARGV

	foreach cfg [list BASIC] {
        set cname $cfg\_$ext
        define_hls_config tf_mult3 $cname -io_config IOCFG_$ext \
            $COMMON_HLS_FLAGS -DHLS_DIRECTIVES_$cfg

		if {$TECH_IS_XILINX == 1} {
		    define_sim_config "$cname\_V" "tf_mult3 RTL_V $cname" "tb TESTBENCH_$ext" \
                -io_config IOCFG_$ext -argv $DEFAULT_ARGV -verilog_top_modules glbl
		} else {
		    define_sim_config "$cname\_V" "tf_mult3 RTL_V $cname" "tb TESTBENCH_$ext" \
                -io_config IOCFG_$ext -argv $DEFAULT_ARGV
		}
	}
}

#
# Compile Flags
#
set_attr hls_cc_options "$INCLUDES"

#
# Simulation Options
#
use_systemc_simulator xcelium
set_attr cc_options "$INCLUDES -DCLOCK_PERIOD=$SIM_CLOCK_PERIOD"
# enable_waveform_logging -vcd
set_attr end_of_sim_command "make saySimPassed"