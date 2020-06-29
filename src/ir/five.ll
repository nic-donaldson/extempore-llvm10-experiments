%mzone = type {i8*, i64, i64, i64, i8*, %mzone*}
%clsvar = type {i8*, i32, i8*, %clsvar*}

declare i8* @llvm_zone_malloc(%mzone*, i64) nounwind
declare void @llvm_zone_mark(%mzone*) nounwind
declare %clsvar* @new_address_table() nounwind
declare %clsvar* @add_address_table(%mzone*, i8*, i32, i8*, i32, %clsvar*)
declare dllexport fastcc i64 @add1_adhoc_W2k2NCxpNjRd__3881(i8* %_impz,i8* %_impenv, i64 %x) nounwind
declare i64 @llvm_zone_mark_size(%mzone*) nounwind
declare void @llvm_zone_ptr_set_size(i8*, i64) nounwind

@gs259 = external global [24 x i8]
@gs260 = external global [35 x i8]

;define dllexport ccc {i8*, i8*, i64 (i8*, i8*, i64)*}** @add1_adhoc_W2k2NCxpNjRd_maker(i8* %_impz) nounwind {
define dllexport ccc {i8*, i8*, i64 (i8*, i8*, i64)*}** @add1_adhoc_W2k2NCxpNjRd_maker(i8* %_impz) nounwind {
entry:
%_impzPtr = alloca i8*
store i8* %_impz, i8** %_impzPtr
%tzone3904 = load i8*, i8** %_impzPtr
%zone3905 = bitcast i8* %tzone3904 to %mzone*

; let assign value to symbol add1_adhoc_W2k2NCxpNjRd
%dat_add1_adhoc_W2k2NCxpNjRd = call i8* @llvm_zone_malloc(%mzone* %zone3905, i64 8)
%add1_adhoc_W2k2NCxpNjRdPtr = bitcast i8* %dat_add1_adhoc_W2k2NCxpNjRd to { i8*, i8*, i64 (i8*, i8*, i64)*}***
%tzone3885 = load i8*, i8** %_impzPtr
%zone3886 = bitcast i8* %tzone3885 to %mzone*
call void @llvm_zone_mark(%mzone* %zone3886)
; malloc closure structure
%clsptr3887 = call i8* @llvm_zone_malloc(%mzone* %zone3886, i64 24)
%closure3888 = bitcast i8* %clsptr3887 to { i8*, i8*, i64 (i8*, i8*, i64)*}*

; malloc environment structure
%envptr3889 = call i8* @llvm_zone_malloc(%mzone* %zone3886, i64 8)
%environment3890 = bitcast i8* %envptr3889 to {{i8*, i8*, i64 (i8*, i8*, i64)*}***}*

; malloc closure address table
%addytable3891 = call %clsvar* @new_address_table()
%var3892 = bitcast [24 x i8]* @gs259 to i8*
%var3893 = bitcast [35 x i8]* @gs260 to i8*
%addytable3894 = call %clsvar* @add_address_table(%mzone* %zone3886, i8* %var3892, i32 0, i8* %var3893, i32 3, %clsvar* %addytable3891)
%address-table3895 = bitcast %clsvar* %addytable3894 to i8*

; insert table, function and environment into closure struct
%closure.table3898 = getelementptr { i8*, i8*, i64 (i8*, i8*, i64)*}, { i8*, i8*, i64 (i8*, i8*, i64)*}* %closure3888, i32 0, i32 0
store i8* %address-table3895, i8** %closure.table3898
%closure.env3899 = getelementptr { i8*, i8*, i64 (i8*, i8*, i64)*}, { i8*, i8*, i64 (i8*, i8*, i64)*}* %closure3888, i32 0, i32 1
store i8* %envptr3889, i8** %closure.env3899
%closure.func3900 = getelementptr { i8*, i8*, i64 (i8*, i8*, i64)*}, { i8*, i8*, i64 (i8*, i8*, i64)*}* %closure3888, i32 0, i32 2
store i64 (i8*, i8*, i64)* @add1_adhoc_W2k2NCxpNjRd__3881, i64 (i8*, i8*, i64)** %closure.func3900
%closure_size3901 = call i64 @llvm_zone_mark_size(%mzone* %zone3886)
call void @llvm_zone_ptr_set_size(i8* %clsptr3887, i64 %closure_size3901)
%wrapper_ptr3902 = call i8* @llvm_zone_malloc(%mzone* %zone3886, i64 8)
%closure_wrapper3903 = bitcast i8* %wrapper_ptr3902 to { i8*, i8*, i64 (i8*, i8*, i64)*}**
store { i8*, i8*, i64 (i8*, i8*, i64)*}* %closure3888, { i8*, i8*, i64 (i8*, i8*, i64)*}** %closure_wrapper3903

; let value assignment
%add1_adhoc_W2k2NCxpNjRd = select i1 true, { i8*, i8*, i64 (i8*, i8*, i64)*}** %closure_wrapper3903, { i8*, i8*, i64 (i8*, i8*, i64)*}** %closure_wrapper3903
store { i8*, i8*, i64 (i8*, i8*, i64)*}** %add1_adhoc_W2k2NCxpNjRd, { i8*, i8*, i64 (i8*, i8*, i64)*}*** %add1_adhoc_W2k2NCxpNjRdPtr

; add data to environment
; don't need to alloc for env var add1_adhoc_W2k2NCxpNjRd
%tmp_envptr3897 = getelementptr {{i8*, i8*, i64 (i8*, i8*, i64)*}***}, {{i8*, i8*, i64 (i8*, i8*, i64)*}***}* %environment3890, i32 0, i32 0
store {i8*, i8*, i64 (i8*, i8*, i64)*}*** %add1_adhoc_W2k2NCxpNjRdPtr, {i8*, i8*, i64 (i8*, i8*, i64)*}**** %tmp_envptr3897


%val3906 = load {i8*, i8*, i64 (i8*, i8*, i64)*}**, {i8*, i8*, i64 (i8*, i8*, i64)*}*** %add1_adhoc_W2k2NCxpNjRdPtr
ret {i8*, i8*, i64 (i8*, i8*, i64)*}** %val3906
}
