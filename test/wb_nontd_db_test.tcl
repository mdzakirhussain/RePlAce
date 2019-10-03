# 
# Examples for Timing-driven RePlAce with TCL usage
#


set design wb_dma_top 
set lib_dir ./library/a2a/
set design_dir ./design/a2a/${design}

replace_external rep

# Import LEF/DEF files
rep import_lef ${lib_dir}/contest.lef
rep import_def ${design_dir}/${design}_mixed_row.def
rep set_output ./output/

rep set_bin_grid_count 64

# Initialize RePlAce
rep init_replace_db

# # place_cell with Nesterov method 
# rep place_cell_nesterov_place
# 
# # Export DEF file
# rep export_def ./output_wb_nontd.def
# 
# puts "nesterovPlace HPWL: [rep get_hpwl]"
# puts "final WNS: [rep get_wns]"
# puts "final TNS: [rep get_tns]"
# 
# exit
