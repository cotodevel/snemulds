/*
-------------------------------------------------------------------
Snezziboy v0.28

Copyright (C) 2006 bubble2k

This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License as 
published by the Free Software Foundation; either version 2 of 
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
GNU General Public License for more details.
-------------------------------------------------------------------
*/

    .equ    memoryMapBase,      0x06898000              @ 8k in VRAM Bank H
@  	.equ    memoryMapBase,      0x023E0000
    .equ    MemoryMap,          memoryMapBase           @ for code compatibility
    .equ	MemoryWriteMap,		0x0689A000
    
/*    .equ    snesWramBase,       0x027C0000*/
	.equ    snesWramBase,       0x023C0000              
/*    .equ    snesWramBase,       0x02200000              @ either 0x2100000 or 0x6800000*/
/*    .equ    snesVramBase,       0x02220000*/


  .include    "../opc_macros.s"


@*************************************************************************
@ All code here in IWRAM
@*************************************************************************

    .section    .dtcm, "aw", %progbits


@=========================================================================
@ memory mapping table
@========================================================================= 
@ version 0.27DS fix

@-------------------------------------------------------------------
@ Decoder Table 
@-------------------------------------------------------------------
m0x0Decoder:
    .long BRK_m0x0,S_28,      ORA_m0x0,DXI_26,    COP_m0x0,S_28,      ORA_m0x0,DS_24,     TSB_m0x0,D_25,      ORA_m0x0,D_23,      ASL_m0x0,D_25,      ORA_m0x0,DIL_26     
    .long PHP_m0x0,PHP_m0x0,  ORA_m0x0I,ORA_m0x0I,ASLA_m0x0,ASLA_m0x0,PHD_m0x0,PHD_m0x0,  TSB_m0x0,A_36,      ORA_m0x0,A_34,      ASL_m0x0,A_36,      ORA_m0x0,AL_45      
    .long BPL_m0x0,BPL_m0x0,  ORA_m0x0,DIY_25,    ORA_m0x0,DI_25,     ORA_m0x0,DSIY_27,   TRB_m0x0,D_25,      ORA_m0x0,DX_24,     ASL_m0x0,DX_26,     ORA_m0x0,DILY_26    
    .long CLC_m0x0,CLC_m0x0,  ORA_m0x0,AY_34,     INCA_m0x0,INCA_m0x0,TCS_m0x0,TCS_m0x0,  TRB_m0x0,A_36,      ORA_m0x0,AX_34,     ASL_m0x0,AX_37,     ORA_m0x0,ALX_45     
    .long JSR_m0x0A,JSR_m0x0A,AND_m0x0,DXI_26,    JSL_m0x0,JSL_m0x0,  AND_m0x0,DS_24,     BIT_m0x0,D_23,      AND_m0x0,D_23,      ROL_m0x0,D_25,      AND_m0x0,DIL_26     
    .long PLP_m0x0,PLP_m0x0,  AND_m0x0I,AND_m0x0I,ROLA_m0x0,ROLA_m0x0,PLD_m0x0,PLD_m0x0,  BIT_m0x0,A_34,      AND_m0x0,A_34,      ROL_m0x0,A_36,      AND_m0x0,AL_45      
    .long BMI_m0x0,BMI_m0x0,  AND_m0x0,DIY_25,    AND_m0x0,DI_25,     AND_m0x0,DSIY_27,   BIT_m0x0,DX_24,     AND_m0x0,DX_24,     ROL_m0x0,DX_26,     AND_m0x0,DILY_26    
    .long SEC_m0x0,SEC_m0x0,  AND_m0x0,AY_34,     DECA_m0x0,DECA_m0x0,TSC_m0x0,TSC_m0x0,  BIT_m0x0,AX_34,     AND_m0x0,AX_34,     ROL_m0x0,AX_37,     AND_m0x0,ALX_45     
    .long RTI_m0x0,RTI_m0x0,  EOR_m0x0,DXI_26,    RES_m0x0,RES_m0x0,  EOR_m0x0,DS_24,     MVP_m0x0,MVP_m0x0,  EOR_m0x0,D_23,      LSR_m0x0,D_25,      EOR_m0x0,DIL_26     
    .long PHA_m0x0,PHA_m0x0,  EOR_m0x0I,EOR_m0x0I,LSRA_m0x0,LSRA_m0x0,PHK_m0x0,PHK_m0x0,  JMP_m0x0,A_J33,     EOR_m0x0,A_34,      LSR_m0x0,A_36,      EOR_m0x0,AL_45      
    .long BVC_m0x0,BVC_m0x0,  EOR_m0x0,DIY_25,    EOR_m0x0,DI_25,     EOR_m0x0,DSIY_27,   MVN_m0x0,MVN_m0x0,  EOR_m0x0,DX_24,     LSR_m0x0,DX_26,     EOR_m0x0,DILY_26    
    .long CLI_m0x0,CLI_m0x0,  EOR_m0x0,AY_34,     PHY_m0x0,PHY_m0x0,  TCD_m0x0,TCD_m0x0,  JMP_m0x0,AL_J44,    EOR_m0x0,AX_34,     LSR_m0x0,AX_37,     EOR_m0x0,ALX_45     
/*6X*/
    .long RTS_m0x0,RTS_m0x0,  ADC_m0x0,DXI_26,    PER_m0x0,PER_m0x0,  ADC_m0x0,DS_24,     STZ_m0x0,D_23,      ADC_m0x0,D_23,      ROR_m0x0,D_25,      ADC_m0x0,DIL_26     
    .long PLA_m0x0,PLA_m0x0,  ADC_m0x0I,ADC_m0x0I,RORA_m0x0,RORA_m0x0,RTL_m0x0,RTL_m0x0,  JMP_m0x0,AI_J35,    ADC_m0x0,A_34,      ROR_m0x0,A_36,      ADC_m0x0,AL_45      
    .long BVS_m0x0,BVS_m0x0,  ADC_m0x0,DIY_25,    ADC_m0x0,DI_25,     ADC_m0x0,DSIY_27,   STZ_m0x0,DX_24,     ADC_m0x0,DX_24,     ROR_m0x0,DX_26,     ADC_m0x0,DILY_26    
    .long SEI_m0x0,SEI_m0x0,  ADC_m0x0,AY_34,     PLY_m0x0,PLY_m0x0,  TDC_m0x0,TDC_m0x0,  JMP_m0x0,AXI_J36,   ADC_m0x0,AX_34,     ROR_m0x0,AX_37,     ADC_m0x0,ALX_45     
    .long BRA_m0x0,BRA_m0x0,  STA_m0x0,DXI_26,    BRL_m0x0,BRL_m0x0,  STA_m0x0,DS_24,     STY_m0x0,D_23,      STAD_m0x0,STAD_m0x0,STX_m0x0,D_23,      STA_m0x0,DIL_26     
    .long DEY_m0x0,DEY_m0x0,  BIT_m0x0I,BIT_m0x0I,TXA_m0x0,TXA_m0x0,  PHB_m0x0,PHB_m0x0,  STY_m0x0,A_34,      STA_m0x0,A_34,      STX_m0x0,A_34,      STA_m0x0,AL_45      
    .long BCC_m0x0,BCC_m0x0,  STA_m0x0,DIY_26,    STA_m0x0,DI_25,     STA_m0x0,DSIY_27,   STY_m0x0,DX_24,     STA_m0x0,DX_24,     STX_m0x0,DY_24,     STA_m0x0,DILY_26    
    .long TYA_m0x0,TYA_m0x0,  STA_m0x0,AY_35,     TXS_m0x0,TXS_m0x0,  TXY_m0x0,TXY_m0x0,  STZ_m0x0,A_34W,     STA_m0x0,AX_35,     STZ_m0x0,AX_35,     STA_m0x0,ALX_45     
    .long LDY_m0x0I,LDY_m0x0I,LDA_m0x0,DXI_26,    LDX_m0x0I,LDX_m0x0I,LDA_m0x0,DS_24,     LDY_m0x0,D_23,      LDAD_m0x0,LDAD_m0x0,LDX_m0x0,D_23,      LDA_m0x0,DIL_26     
    .long TAY_m0x0,TAY_m0x0,  LDA_m0x0I,LDA_m0x0I,TAX_m0x0,TAX_m0x0,  PLB_m0x0,PLB_m0x0,  LDY_m0x0,A_34,      LDAA_m0x0,LDAA_m0x0,LDX_m0x0,A_34,      LDA_m0x0,AL_45      
    .long BCS_m0x0,BCS_m0x0,  LDA_m0x0,DIY_25,    LDA_m0x0,DI_25,     LDA_m0x0,DSIY_27,   LDY_m0x0,DX_24,     LDA_m0x0,DX_24,     LDX_m0x0,DY_24,     LDA_m0x0,DILY_26    
    .long CLV_m0x0,CLV_m0x0,  LDA_m0x0,AY_34,     TSX_m0x0,TSX_m0x0,  TYX_m0x0,TYX_m0x0,  LDY_m0x0,AX_34,     LDA_m0x0,AX_34,     LDX_m0x0,AY_34,     LDA_m0x0,ALX_45     
    .long CPY_m0x0I,CPY_m0x0I,CMP_m0x0,DXI_26,    REP_m0x0,REP_m0x0,  CMP_m0x0,DS_24,     CPY_m0x0,D_23,      CMP_m0x0,D_23,      DEC_m0x0,D_25,      CMP_m0x0,DIL_26     
    .long INY_m0x0,INY_m0x0,  CMP_m0x0I,CMP_m0x0I,DEX_m0x0,DEX_m0x0,  WAI_m0x0,WAI_m0x0,  CPY_m0x0,A_34,      CMP_m0x0,A_34,      DEC_m0x0,A_34,      CMP_m0x0,AL_45      
    .long BNE_m0x0,BNE_m0x0,  CMP_m0x0,DIY_25,    CMP_m0x0,DI_25,     CMP_m0x0,DSIY_27,   PEI_m0x0,PEI_m0x0,  CMP_m0x0,DX_24,     DEC_m0x0,DX_26,     CMP_m0x0,DILY_26    
    .long CLD_m0x0,CLD_m0x0,  CMP_m0x0,AY_34,     PHX_m0x0,PHX_m0x0,  STP_m0x0,STP_m0x0,  JML_m0x0,AIL_J36,   CMP_m0x0,AX_34,     DEC_m0x0,AX_37,     CMP_m0x0,ALX_45     
    .long CPX_m0x0I,CPX_m0x0I,SBC_m0x0,DXI_26,    SEP_m0x0,SEP_m0x0,  SBC_m0x0,DS_24,     CPX_m0x0,D_23,      SBC_m0x0,D_23,      INC_m0x0,D_25,      SBC_m0x0,DIL_26     
    .long INX_m0x0,INX_m0x0,  SBC_m0x0I,SBC_m0x0I,NOP_m0x0,NOP_m0x0,  XBA_m0x0,XBA_m0x0,  CPX_m0x0,A_34,      SBC_m0x0,A_34,      INC_m0x0,A_36,      SBC_m0x0,AL_45      
    .long BEQ_m0x0,BEQ_m0x0,  SBC_m0x0,DIY_25,    SBC_m0x0,DI_25,     SBC_m0x0,DSIY_27,   PEA_m0x0,PEA_m0x0,  SBC_m0x0,DX_24,     INC_m0x0,DX_26,     SBC_m0x0,DILY_26    
    .long SED_m0x0,SED_m0x0,  SBC_m0x0,AY_34,     PLX_m0x0,PLX_m0x0,  XCE_m0x0,XCE_m0x0,  JSR_m0x0I,JSR_m0x0I,SBC_m0x0,AX_34,     INC_m0x0,AX_37,     SBC_m0x0,ALX_45     
m0x1Decoder:
    .long BRK_m0x1,S_28,      ORA_m0x1,DXI_26,    COP_m0x1,S_28,      ORA_m0x1,DS_24,     TSB_m0x1,D_25,      ORA_m0x1,D_23,      ASL_m0x1,D_25,      ORA_m0x1,DIL_26     
    .long PHP_m0x1,PHP_m0x1,  ORA_m0x1I,ORA_m0x1I,ASLA_m0x1,ASLA_m0x1,PHD_m0x1,PHD_m0x1,  TSB_m0x1,A_36,      ORA_m0x1,A_34,      ASL_m0x1,A_36,      ORA_m0x1,AL_45      
    .long BPL_m0x1,BPL_m0x1,  ORA_m0x1,DIY_25,    ORA_m0x1,DI_25,     ORA_m0x1,DSIY_27,   TRB_m0x1,D_25,      ORA_m0x1,DX_24,     ASL_m0x1,DX_26,     ORA_m0x1,DILY_26    
    .long CLC_m0x1,CLC_m0x1,  ORA_m0x1,AY_34,     INCA_m0x1,INCA_m0x1,TCS_m0x1,TCS_m0x1,  TRB_m0x1,A_36,      ORA_m0x1,AX_34,     ASL_m0x1,AX_37,     ORA_m0x1,ALX_45     
    .long JSR_m0x1A,JSR_m0x1A,AND_m0x1,DXI_26,    JSL_m0x1,JSL_m0x1,  AND_m0x1,DS_24,     BIT_m0x1,D_23,      AND_m0x1,D_23,      ROL_m0x1,D_25,      AND_m0x1,DIL_26     
    .long PLP_m0x1,PLP_m0x1,  AND_m0x1I,AND_m0x1I,ROLA_m0x1,ROLA_m0x1,PLD_m0x1,PLD_m0x1,  BIT_m0x1,A_34,      AND_m0x1,A_34,      ROL_m0x1,A_36,      AND_m0x1,AL_45      
    .long BMI_m0x1,BMI_m0x1,  AND_m0x1,DIY_25,    AND_m0x1,DI_25,     AND_m0x1,DSIY_27,   BIT_m0x1,DX_24,     AND_m0x1,DX_24,     ROL_m0x1,DX_26,     AND_m0x1,DILY_26    
    .long SEC_m0x1,SEC_m0x1,  AND_m0x1,AY_34,     DECA_m0x1,DECA_m0x1,TSC_m0x1,TSC_m0x1,  BIT_m0x1,AX_34,     AND_m0x1,AX_34,     ROL_m0x1,AX_37,     AND_m0x1,ALX_45     
    .long RTI_m0x1,RTI_m0x1,  EOR_m0x1,DXI_26,    RES_m0x1,RES_m0x1,  EOR_m0x1,DS_24,     MVP_m0x1,MVP_m0x1,  EOR_m0x1,D_23,      LSR_m0x1,D_25,      EOR_m0x1,DIL_26     
    .long PHA_m0x1,PHA_m0x1,  EOR_m0x1I,EOR_m0x1I,LSRA_m0x1,LSRA_m0x1,PHK_m0x1,PHK_m0x1,  JMP_m0x1,A_J33,     EOR_m0x1,A_34,      LSR_m0x1,A_36,      EOR_m0x1,AL_45      
    .long BVC_m0x1,BVC_m0x1,  EOR_m0x1,DIY_25,    EOR_m0x1,DI_25,     EOR_m0x1,DSIY_27,   MVN_m0x1,MVN_m0x1,  EOR_m0x1,DX_24,     LSR_m0x1,DX_26,     EOR_m0x1,DILY_26    
    .long CLI_m0x1,CLI_m0x1,  EOR_m0x1,AY_34,     PHY_m0x1,PHY_m0x1,  TCD_m0x1,TCD_m0x1,  JMP_m0x1,AL_J44,    EOR_m0x1,AX_34,     LSR_m0x1,AX_37,     EOR_m0x1,ALX_45     
    .long RTS_m0x1,RTS_m0x1,  ADC_m0x1,DXI_26,    PER_m0x1,PER_m0x1,  ADC_m0x1,DS_24,     STZ_m0x1,D_23,      ADC_m0x1,D_23,      ROR_m0x1,D_25,      ADC_m0x1,DIL_26     
    .long PLA_m0x1,PLA_m0x1,  ADC_m0x1I,ADC_m0x1I,RORA_m0x1,RORA_m0x1,RTL_m0x1,RTL_m0x1,  JMP_m0x1,AI_J35,    ADC_m0x1,A_34,      ROR_m0x1,A_36,      ADC_m0x1,AL_45      
    .long BVS_m0x1,BVS_m0x1,  ADC_m0x1,DIY_25,    ADC_m0x1,DI_25,     ADC_m0x1,DSIY_27,   STZ_m0x1,DX_24,     ADC_m0x1,DX_24,     ROR_m0x1,DX_26,     ADC_m0x1,DILY_26    
    .long SEI_m0x1,SEI_m0x1,  ADC_m0x1,AY_34,     PLY_m0x1,PLY_m0x1,  TDC_m0x1,TDC_m0x1,  JMP_m0x1,AXI_J36,   ADC_m0x1,AX_34,     ROR_m0x1,AX_37,     ADC_m0x1,ALX_45     
    .long BRA_m0x1,BRA_m0x1,  STA_m0x1,DXI_26,    BRL_m0x1,BRL_m0x1,  STA_m0x1,DS_24,     STY_m0x1,D_23,      STAD_m0x1,STAD_m0x1,STX_m0x1,D_23,      STA_m0x1,DIL_26     
    .long DEY_m0x1,DEY_m0x1,  BIT_m0x1I,BIT_m0x1I,TXA_m0x1,TXA_m0x1,  PHB_m0x1,PHB_m0x1,  STY_m0x1,A_34,      STA_m0x1,A_34,      STX_m0x1,A_34,      STA_m0x1,AL_45      
    .long BCC_m0x1,BCC_m0x1,  STA_m0x1,DIY_26,    STA_m0x1,DI_25,     STA_m0x1,DSIY_27,   STY_m0x1,DX_24,     STA_m0x1,DX_24,     STX_m0x1,DY_24,     STA_m0x1,DILY_26    
    .long TYA_m0x1,TYA_m0x1,  STA_m0x1,AY_35,     TXS_m0x1,TXS_m0x1,  TXY_m0x1,TXY_m0x1,  STZ_m0x1,A_34W,     STA_m0x1,AX_35,     STZ_m0x1,AX_35,     STA_m0x1,ALX_45     
    .long LDY_m0x1I,LDY_m0x1I,LDA_m0x1,DXI_26,    LDX_m0x1I,LDX_m0x1I,LDA_m0x1,DS_24,     LDY_m0x1,D_23,      LDAD_m0x1,LDAD_m0x1,LDX_m0x1,D_23,      LDA_m0x1,DIL_26     
    .long TAY_m0x1,TAY_m0x1,  LDA_m0x1I,LDA_m0x1I,TAX_m0x1,TAX_m0x1,  PLB_m0x1,PLB_m0x1,  LDY_m0x1,A_34,      LDAA_m0x1,LDAA_m0x1,LDX_m0x1,A_34,      LDA_m0x1,AL_45      
    .long BCS_m0x1,BCS_m0x1,  LDA_m0x1,DIY_25,    LDA_m0x1,DI_25,     LDA_m0x1,DSIY_27,   LDY_m0x1,DX_24,     LDA_m0x1,DX_24,     LDX_m0x1,DY_24,     LDA_m0x1,DILY_26    
    .long CLV_m0x1,CLV_m0x1,  LDA_m0x1,AY_34,     TSX_m0x1,TSX_m0x1,  TYX_m0x1,TYX_m0x1,  LDY_m0x1,AX_34,     LDA_m0x1,AX_34,     LDX_m0x1,AY_34,     LDA_m0x1,ALX_45     
    .long CPY_m0x1I,CPY_m0x1I,CMP_m0x1,DXI_26,    REP_m0x1,REP_m0x1,  CMP_m0x1,DS_24,     CPY_m0x1,D_23,      CMP_m0x1,D_23,      DEC_m0x1,D_25,      CMP_m0x1,DIL_26     
    .long INY_m0x1,INY_m0x1,  CMP_m0x1I,CMP_m0x1I,DEX_m0x1,DEX_m0x1,  WAI_m0x1,WAI_m0x1,  CPY_m0x1,A_34,      CMP_m0x1,A_34,      DEC_m0x1,A_34,      CMP_m0x1,AL_45      
    .long BNE_m0x1,BNE_m0x1,  CMP_m0x1,DIY_25,    CMP_m0x1,DI_25,     CMP_m0x1,DSIY_27,   PEI_m0x1,PEI_m0x1,  CMP_m0x1,DX_24,     DEC_m0x1,DX_26,     CMP_m0x1,DILY_26    
    .long CLD_m0x1,CLD_m0x1,  CMP_m0x1,AY_34,     PHX_m0x1,PHX_m0x1,  STP_m0x1,STP_m0x1,  JML_m0x1,AIL_J36,   CMP_m0x1,AX_34,     DEC_m0x1,AX_37,     CMP_m0x1,ALX_45     
    .long CPX_m0x1I,CPX_m0x1I,SBC_m0x1,DXI_26,    SEP_m0x1,SEP_m0x1,  SBC_m0x1,DS_24,     CPX_m0x1,D_23,      SBC_m0x1,D_23,      INC_m0x1,D_25,      SBC_m0x1,DIL_26     
    .long INX_m0x1,INX_m0x1,  SBC_m0x1I,SBC_m0x1I,NOP_m0x1,NOP_m0x1,  XBA_m0x1,XBA_m0x1,  CPX_m0x1,A_34,      SBC_m0x1,A_34,      INC_m0x1,A_36,      SBC_m0x1,AL_45      
    .long BEQ_m0x1,BEQ_m0x1,  SBC_m0x1,DIY_25,    SBC_m0x1,DI_25,     SBC_m0x1,DSIY_27,   PEA_m0x1,PEA_m0x1,  SBC_m0x1,DX_24,     INC_m0x1,DX_26,     SBC_m0x1,DILY_26    
    .long SED_m0x1,SED_m0x1,  SBC_m0x1,AY_34,     PLX_m0x1,PLX_m0x1,  XCE_m0x1,XCE_m0x1,  JSR_m0x1I,JSR_m0x1I,SBC_m0x1,AX_34,     INC_m0x1,AX_37,     SBC_m0x1,ALX_45     
m1x0Decoder:
    .long BRK_m1x0,S_28,      ORA_m1x0,DXI_26,    COP_m1x0,S_28,      ORA_m1x0,DS_24,     TSB_m1x0,D_25,      ORA_m1x0,D_23,      ASL_m1x0,D_25,      ORA_m1x0,DIL_26     
    .long PHP_m1x0,PHP_m1x0,  ORA_m1x0I,ORA_m1x0I,ASLA_m1x0,ASLA_m1x0,PHD_m1x0,PHD_m1x0,  TSB_m1x0,A_36,      ORA_m1x0,A_34,      ASL_m1x0,A_36,      ORA_m1x0,AL_45      
    .long BPL_m1x0,BPL_m1x0,  ORA_m1x0,DIY_25,    ORA_m1x0,DI_25,     ORA_m1x0,DSIY_27,   TRB_m1x0,D_25,      ORA_m1x0,DX_24,     ASL_m1x0,DX_26,     ORA_m1x0,DILY_26    
    .long CLC_m1x0,CLC_m1x0,  ORA_m1x0,AY_34,     INCA_m1x0,INCA_m1x0,TCS_m1x0,TCS_m1x0,  TRB_m1x0,A_36,      ORA_m1x0,AX_34,     ASL_m1x0,AX_37,     ORA_m1x0,ALX_45     
    .long JSR_m1x0A,JSR_m1x0A,AND_m1x0,DXI_26,    JSL_m1x0,JSL_m1x0,  AND_m1x0,DS_24,     BIT_m1x0,D_23,      AND_m1x0,D_23,      ROL_m1x0,D_25,      AND_m1x0,DIL_26     
    .long PLP_m1x0,PLP_m1x0,  AND_m1x0I,AND_m1x0I,ROLA_m1x0,ROLA_m1x0,PLD_m1x0,PLD_m1x0,  BIT_m1x0,A_34,      AND_m1x0,A_34,      ROL_m1x0,A_36,      AND_m1x0,AL_45      
    .long BMI_m1x0,BMI_m1x0,  AND_m1x0,DIY_25,    AND_m1x0,DI_25,     AND_m1x0,DSIY_27,   BIT_m1x0,DX_24,     AND_m1x0,DX_24,     ROL_m1x0,DX_26,     AND_m1x0,DILY_26    
    .long SEC_m1x0,SEC_m1x0,  AND_m1x0,AY_34,     DECA_m1x0,DECA_m1x0,TSC_m1x0,TSC_m1x0,  BIT_m1x0,AX_34,     AND_m1x0,AX_34,     ROL_m1x0,AX_37,     AND_m1x0,ALX_45     
    .long RTI_m1x0,RTI_m1x0,  EOR_m1x0,DXI_26,    RES_m1x0,RES_m1x0,  EOR_m1x0,DS_24,     MVP_m1x0,MVP_m1x0,  EOR_m1x0,D_23,      LSR_m1x0,D_25,      EOR_m1x0,DIL_26     
    .long PHA_m1x0,PHA_m1x0,  EOR_m1x0I,EOR_m1x0I,LSRA_m1x0,LSRA_m1x0,PHK_m1x0,PHK_m1x0,  JMP_m1x0,A_J33,     EOR_m1x0,A_34,      LSR_m1x0,A_36,      EOR_m1x0,AL_45      
    .long BVC_m1x0,BVC_m1x0,  EOR_m1x0,DIY_25,    EOR_m1x0,DI_25,     EOR_m1x0,DSIY_27,   MVN_m1x0,MVN_m1x0,  EOR_m1x0,DX_24,     LSR_m1x0,DX_26,     EOR_m1x0,DILY_26    
    .long CLI_m1x0,CLI_m1x0,  EOR_m1x0,AY_34,     PHY_m1x0,PHY_m1x0,  TCD_m1x0,TCD_m1x0,  JMP_m1x0,AL_J44,    EOR_m1x0,AX_34,     LSR_m1x0,AX_37,     EOR_m1x0,ALX_45     
    .long RTS_m1x0,RTS_m1x0,  ADC_m1x0,DXI_26,    PER_m1x0,PER_m1x0,  ADC_m1x0,DS_24,     STZ_m1x0,D_23,      ADC_m1x0,D_23,      ROR_m1x0,D_25,      ADC_m1x0,DIL_26     
    .long PLA_m1x0,PLA_m1x0,  ADC_m1x0I,ADC_m1x0I,RORA_m1x0,RORA_m1x0,RTL_m1x0,RTL_m1x0,  JMP_m1x0,AI_J35,    ADC_m1x0,A_34,      ROR_m1x0,A_36,      ADC_m1x0,AL_45      
    .long BVS_m1x0,BVS_m1x0,  ADC_m1x0,DIY_25,    ADC_m1x0,DI_25,     ADC_m1x0,DSIY_27,   STZ_m1x0,DX_24,     ADC_m1x0,DX_24,     ROR_m1x0,DX_26,     ADC_m1x0,DILY_26    
    .long SEI_m1x0,SEI_m1x0,  ADC_m1x0,AY_34,     PLY_m1x0,PLY_m1x0,  TDC_m1x0,TDC_m1x0,  JMP_m1x0,AXI_J36,   ADC_m1x0,AX_34,     ROR_m1x0,AX_37,     ADC_m1x0,ALX_45     
    .long BRA_m1x0,BRA_m1x0,  STA_m1x0,DXI_26,    BRL_m1x0,BRL_m1x0,  STA_m1x0,DS_24,     STY_m1x0,D_23,      STAD_m1x0,STAD_m1x0,STX_m1x0,D_23,      STA_m1x0,DIL_26     
    .long DEY_m1x0,DEY_m1x0,  BIT_m1x0I,BIT_m1x0I,TXA_m1x0,TXA_m1x0,  PHB_m1x0,PHB_m1x0,  STY_m1x0,A_34,      STA_m1x0,A_34,      STX_m1x0,A_34,      STA_m1x0,AL_45      
    .long BCC_m1x0,BCC_m1x0,  STA_m1x0,DIY_26,    STA_m1x0,DI_25,     STA_m1x0,DSIY_27,   STY_m1x0,DX_24,     STA_m1x0,DX_24,     STX_m1x0,DY_24,     STA_m1x0,DILY_26    
    .long TYA_m1x0,TYA_m1x0,  STA_m1x0,AY_35,     TXS_m1x0,TXS_m1x0,  TXY_m1x0,TXY_m1x0,  STZ_m1x0,A_34W,     STA_m1x0,AX_35,     STZ_m1x0,AX_35,     STA_m1x0,ALX_45     
    .long LDY_m1x0I,LDY_m1x0I,LDA_m1x0,DXI_26,    LDX_m1x0I,LDX_m1x0I,LDA_m1x0,DS_24,     LDY_m1x0,D_23,      LDAD_m1x0,LDAD_m1x0,LDX_m1x0,D_23,      LDA_m1x0,DIL_26     
    .long TAY_m1x0,TAY_m1x0,  LDA_m1x0I,LDA_m1x0I,TAX_m1x0,TAX_m1x0,  PLB_m1x0,PLB_m1x0,  LDY_m1x0,A_34,      LDAA_m1x0,LDAA_m1x0,LDX_m1x0,A_34,      LDA_m1x0,AL_45      
    .long BCS_m1x0,BCS_m1x0,  LDA_m1x0,DIY_25,    LDA_m1x0,DI_25,     LDA_m1x0,DSIY_27,   LDY_m1x0,DX_24,     LDA_m1x0,DX_24,     LDX_m1x0,DY_24,     LDA_m1x0,DILY_26    
    .long CLV_m1x0,CLV_m1x0,  LDA_m1x0,AY_34,     TSX_m1x0,TSX_m1x0,  TYX_m1x0,TYX_m1x0,  LDY_m1x0,AX_34,     LDA_m1x0,AX_34,     LDX_m1x0,AY_34,     LDA_m1x0,ALX_45     
    .long CPY_m1x0I,CPY_m1x0I,CMP_m1x0,DXI_26,    REP_m1x0,REP_m1x0,  CMP_m1x0,DS_24,     CPY_m1x0,D_23,      CMP_m1x0,D_23,      DEC_m1x0,D_25,      CMP_m1x0,DIL_26     
    .long INY_m1x0,INY_m1x0,  CMP_m1x0I,CMP_m1x0I,DEX_m1x0,DEX_m1x0,  WAI_m1x0,WAI_m1x0,  CPY_m1x0,A_34,      CMP_m1x0,A_34,      DEC_m1x0,A_34,      CMP_m1x0,AL_45      
    .long BNE_m1x0,BNE_m1x0,  CMP_m1x0,DIY_25,    CMP_m1x0,DI_25,     CMP_m1x0,DSIY_27,   PEI_m1x0,PEI_m1x0,  CMP_m1x0,DX_24,     DEC_m1x0,DX_26,     CMP_m1x0,DILY_26    
    .long CLD_m1x0,CLD_m1x0,  CMP_m1x0,AY_34,     PHX_m1x0,PHX_m1x0,  STP_m1x0,STP_m1x0,  JML_m1x0,AIL_J36,   CMP_m1x0,AX_34,     DEC_m1x0,AX_37,     CMP_m1x0,ALX_45     
    .long CPX_m1x0I,CPX_m1x0I,SBC_m1x0,DXI_26,    SEP_m1x0,SEP_m1x0,  SBC_m1x0,DS_24,     CPX_m1x0,D_23,      SBC_m1x0,D_23,      INC_m1x0,D_25,      SBC_m1x0,DIL_26     
    .long INX_m1x0,INX_m1x0,  SBC_m1x0I,SBC_m1x0I,NOP_m1x0,NOP_m1x0,  XBA_m1x0,XBA_m1x0,  CPX_m1x0,A_34,      SBC_m1x0,A_34,      INC_m1x0,A_36,      SBC_m1x0,AL_45      
    .long BEQ_m1x0,BEQ_m1x0,  SBC_m1x0,DIY_25,    SBC_m1x0,DI_25,     SBC_m1x0,DSIY_27,   PEA_m1x0,PEA_m1x0,  SBC_m1x0,DX_24,     INC_m1x0,DX_26,     SBC_m1x0,DILY_26    
    .long SED_m1x0,SED_m1x0,  SBC_m1x0,AY_34,     PLX_m1x0,PLX_m1x0,  XCE_m1x0,XCE_m1x0,  JSR_m1x0I,JSR_m1x0I,SBC_m1x0,AX_34,     INC_m1x0,AX_37,     SBC_m1x0,ALX_45     
m1x1Decoder:
    .long BRK_m1x1,S_28,      ORA_m1x1,DXI_26,    COP_m1x1,S_28,      ORA_m1x1,DS_24,     TSB_m1x1,D_25,      ORA_m1x1,D_23,      ASL_m1x1,D_25,      ORA_m1x1,DIL_26     
    .long PHP_m1x1,PHP_m1x1,  ORA_m1x1I,ORA_m1x1I,ASLA_m1x1,ASLA_m1x1,PHD_m1x1,PHD_m1x1,  TSB_m1x1,A_36,      ORA_m1x1,A_34,      ASL_m1x1,A_36,      ORA_m1x1,AL_45      
    .long BPL_m1x1,BPL_m1x1,  ORA_m1x1,DIY_25,    ORA_m1x1,DI_25,     ORA_m1x1,DSIY_27,   TRB_m1x1,D_25,      ORA_m1x1,DX_24,     ASL_m1x1,DX_26,     ORA_m1x1,DILY_26    
    .long CLC_m1x1,CLC_m1x1,  ORA_m1x1,AY_34,     INCA_m1x1,INCA_m1x1,TCS_m1x1,TCS_m1x1,  TRB_m1x1,A_36,      ORA_m1x1,AX_34,     ASL_m1x1,AX_37,     ORA_m1x1,ALX_45     
    .long JSR_m1x1A,JSR_m1x1A,AND_m1x1,DXI_26,    JSL_m1x1,JSL_m1x1,  AND_m1x1,DS_24,     BIT_m1x1,D_23,      AND_m1x1,D_23,      ROL_m1x1,D_25,      AND_m1x1,DIL_26     
    .long PLP_m1x1,PLP_m1x1,  AND_m1x1I,AND_m1x1I,ROLA_m1x1,ROLA_m1x1,PLD_m1x1,PLD_m1x1,  BIT_m1x1,A_34,      AND_m1x1,A_34,      ROL_m1x1,A_36,      AND_m1x1,AL_45      
    .long BMI_m1x1,BMI_m1x1,  AND_m1x1,DIY_25,    AND_m1x1,DI_25,     AND_m1x1,DSIY_27,   BIT_m1x1,DX_24,     AND_m1x1,DX_24,     ROL_m1x1,DX_26,     AND_m1x1,DILY_26    
    .long SEC_m1x1,SEC_m1x1,  AND_m1x1,AY_34,     DECA_m1x1,DECA_m1x1,TSC_m1x1,TSC_m1x1,  BIT_m1x1,AX_34,     AND_m1x1,AX_34,     ROL_m1x1,AX_37,     AND_m1x1,ALX_45     
    .long RTI_m1x1,RTI_m1x1,  EOR_m1x1,DXI_26,    RES_m1x1,RES_m1x1,  EOR_m1x1,DS_24,     MVP_m1x1,MVP_m1x1,  EOR_m1x1,D_23,      LSR_m1x1,D_25,      EOR_m1x1,DIL_26     
    .long PHA_m1x1,PHA_m1x1,  EOR_m1x1I,EOR_m1x1I,LSRA_m1x1,LSRA_m1x1,PHK_m1x1,PHK_m1x1,  JMP_m1x1,A_J33,     EOR_m1x1,A_34,      LSR_m1x1,A_36,      EOR_m1x1,AL_45      
    .long BVC_m1x1,BVC_m1x1,  EOR_m1x1,DIY_25,    EOR_m1x1,DI_25,     EOR_m1x1,DSIY_27,   MVN_m1x1,MVN_m1x1,  EOR_m1x1,DX_24,     LSR_m1x1,DX_26,     EOR_m1x1,DILY_26    
    .long CLI_m1x1,CLI_m1x1,  EOR_m1x1,AY_34,     PHY_m1x1,PHY_m1x1,  TCD_m1x1,TCD_m1x1,  JMP_m1x1,AL_J44,    EOR_m1x1,AX_34,     LSR_m1x1,AX_37,     EOR_m1x1,ALX_45     
    .long RTS_m1x1,RTS_m1x1,  ADC_m1x1,DXI_26,    PER_m1x1,PER_m1x1,  ADC_m1x1,DS_24,     STZ_m1x1,D_23,      ADC_m1x1,D_23,      ROR_m1x1,D_25,      ADC_m1x1,DIL_26     
    .long PLA_m1x1,PLA_m1x1,  ADC_m1x1I,ADC_m1x1I,RORA_m1x1,RORA_m1x1,RTL_m1x1,RTL_m1x1,  JMP_m1x1,AI_J35,    ADC_m1x1,A_34,      ROR_m1x1,A_36,      ADC_m1x1,AL_45      
    .long BVS_m1x1,BVS_m1x1,  ADC_m1x1,DIY_25,    ADC_m1x1,DI_25,     ADC_m1x1,DSIY_27,   STZ_m1x1,DX_24,     ADC_m1x1,DX_24,     ROR_m1x1,DX_26,     ADC_m1x1,DILY_26    
    .long SEI_m1x1,SEI_m1x1,  ADC_m1x1,AY_34,     PLY_m1x1,PLY_m1x1,  TDC_m1x1,TDC_m1x1,  JMP_m1x1,AXI_J36,   ADC_m1x1,AX_34,     ROR_m1x1,AX_37,     ADC_m1x1,ALX_45     
    .long BRA_m1x1,BRA_m1x1,  STA_m1x1,DXI_26,    BRL_m1x1,BRL_m1x1,  STA_m1x1,DS_24,     STY_m1x1,D_23,      STAD_m1x1,STAD_m1x1,STX_m1x1,D_23,      STA_m1x1,DIL_26     
    .long DEY_m1x1,DEY_m1x1,  BIT_m1x1I,BIT_m1x1I,TXA_m1x1,TXA_m1x1,  PHB_m1x1,PHB_m1x1,  STY_m1x1,A_34,      STA_m1x1,A_34,      STX_m1x1,A_34,      STA_m1x1,AL_45      
    .long BCC_m1x1,BCC_m1x1,  STA_m1x1,DIY_26,    STA_m1x1,DI_25,     STA_m1x1,DSIY_27,   STY_m1x1,DX_24,     STA_m1x1,DX_24,     STX_m1x1,DY_24,     STA_m1x1,DILY_26    
    .long TYA_m1x1,TYA_m1x1,  STA_m1x1,AY_35,     TXS_m1x1,TXS_m1x1,  TXY_m1x1,TXY_m1x1,  STZ_m1x1,A_34W,     STA_m1x1,AX_35,     STZ_m1x1,AX_35,     STA_m1x1,ALX_45     
    .long LDY_m1x1I,LDY_m1x1I,LDA_m1x1,DXI_26,    LDX_m1x1I,LDX_m1x1I,LDA_m1x1,DS_24,     LDY_m1x1,D_23,      LDAD_m1x1,LDAD_m1x1,LDX_m1x1,D_23,      LDA_m1x1,DIL_26     
    .long TAY_m1x1,TAY_m1x1,  LDA_m1x1I,LDA_m1x1I,TAX_m1x1,TAX_m1x1,  PLB_m1x1,PLB_m1x1,  LDY_m1x1,A_34,      LDAA_m1x1,LDAA_m1x1,LDX_m1x1,A_34,      LDA_m1x1,AL_45      
    .long BCS_m1x1,BCS_m1x1,  LDA_m1x1,DIY_25,    LDA_m1x1,DI_25,     LDA_m1x1,DSIY_27,   LDY_m1x1,DX_24,     LDA_m1x1,DX_24,     LDX_m1x1,DY_24,     LDA_m1x1,DILY_26    
    .long CLV_m1x1,CLV_m1x1,  LDA_m1x1,AY_34,     TSX_m1x1,TSX_m1x1,  TYX_m1x1,TYX_m1x1,  LDY_m1x1,AX_34,     LDA_m1x1,AX_34,     LDX_m1x1,AY_34,     LDA_m1x1,ALX_45     
    .long CPY_m1x1I,CPY_m1x1I,CMP_m1x1,DXI_26,    REP_m1x1,REP_m1x1,  CMP_m1x1,DS_24,     CPY_m1x1,D_23,      CMP_m1x1,D_23,      DEC_m1x1,D_25,      CMP_m1x1,DIL_26     
    .long INY_m1x1,INY_m1x1,  CMP_m1x1I,CMP_m1x1I,DEX_m1x1,DEX_m1x1,  WAI_m1x1,WAI_m1x1,  CPY_m1x1,A_34,      CMP_m1x1,A_34,      DEC_m1x1,A_34,      CMP_m1x1,AL_45      
    .long BNE_m1x1,BNE_m1x1,  CMP_m1x1,DIY_25,    CMP_m1x1,DI_25,     CMP_m1x1,DSIY_27,   PEI_m1x1,PEI_m1x1,  CMP_m1x1,DX_24,     DEC_m1x1,DX_26,     CMP_m1x1,DILY_26    
    .long CLD_m1x1,CLD_m1x1,  CMP_m1x1,AY_34,     PHX_m1x1,PHX_m1x1,  STP_m1x1,STP_m1x1,  JML_m1x1,AIL_J36,   CMP_m1x1,AX_34,     DEC_m1x1,AX_37,     CMP_m1x1,ALX_45     
    .long CPX_m1x1I,CPX_m1x1I,SBC_m1x1,DXI_26,    SEP_m1x1,SEP_m1x1,  SBC_m1x1,DS_24,     CPX_m1x1,D_23,      SBC_m1x1,D_23,      INC_m1x1,D_25,      SBC_m1x1,DIL_26     
    .long INX_m1x1,INX_m1x1,  SBC_m1x1I,SBC_m1x1I,NOP_m1x1,NOP_m1x1,  XBA_m1x1,XBA_m1x1,  CPX_m1x1,A_34,      SBC_m1x1,A_34,      INC_m1x1,A_36,      SBC_m1x1,AL_45      
    .long BEQ_m1x1,BEQ_m1x1,  SBC_m1x1,DIY_25,    SBC_m1x1,DI_25,     SBC_m1x1,DSIY_27,   PEA_m1x1,PEA_m1x1,  SBC_m1x1,DX_24,     INC_m1x1,DX_26,     SBC_m1x1,DILY_26    
    .long SED_m1x1,SED_m1x1,  SBC_m1x1,AY_34,     PLX_m1x1,PLX_m1x1,  XCE_m1x1,XCE_m1x1,  JSR_m1x1I,JSR_m1x1I,SBC_m1x1,AX_34,     INC_m1x1,AX_37,     SBC_m1x1,ALX_45     
@ version 0.27DS fix end

    .section    .itcm, "awx", %progbits

    .align 4
/*    .ascii  ".IWRAMSTART"
    .align 4*/
    
@-------------------------------------------------------------------
@ First bank for DP addressing
@-------------------------------------------------------------------
DPCache:
    .rept   16
    .long   0
    .endr


/*
@-------------------------------------------------------------------
@ Caching for memory mapping 
@-------------------------------------------------------------------
MapCacheOffset:
    .long   0

MapCache:
    .rept   16
    .long   0
    .endr
*/

@-------------------------------------------------------------------
@ Special Memory Functions 
@-------------------------------------------------------------------
MemFunctions:
	.rept	16
	.long	0
	.endr


@-------------------------------------------------------------------
@ Addressing Modes 
@-------------------------------------------------------------------
SetRead
A_34W	:
A_34      :  Translate Absolute,3,4
A_36      :  Translate Absolute,3,6

@SetWrite
@A_34W	  :	 Translate Absolute,3,4

SetRead
A_J33     :  Translate AbsolutePC,0,3

AI_J35    :  Translate AbsoluteIndirectPC,0,5
AIL_J36   :  Translate AbsoluteIndirectLongPC,0,6

AL_45     :  Translate AbsoluteLong,4,5

AL_J44    :  Translate AbsoluteLongPC,0,4           @ JML

ALX_45    :  Translate AbsoluteLongIndexedX,4,5

AX_34     :  Translate AbsoluteIndexedX,3,4
AX_35     :  Translate AbsoluteIndexedX,3,5
AX_37     :  Translate AbsoluteIndexedX,3,7

AXI_J36   :  Translate AbsoluteIndexedXIndirectPC,0,6

AY_34     :  Translate AbsoluteIndexedY,3,4
AY_35     :  Translate AbsoluteIndexedY,3,5

D_23      :  Translate DP,2,3
D_25      :  Translate DP,2,5

DX_24     :  Translate DPIndexedX,2,4
DX_26     :  Translate DPIndexedX,2,6

DY_24     :  Translate DPIndexedY,2,4

DI_25     :  Translate DPIndirect,2,5

DIL_26    :  Translate DPIndirectLong,2,6

DIY_25    :  Translate DPIndirectIndexedY,2,5
DIY_26    :  Translate DPIndirectIndexedY,2,6

DILY_26   :  Translate DPIndirectLongIndexedY,2,6

DXI_26    :  Translate DPIndexedXIndirect,2,6

DS_24     :  Translate StackRelative,2,4
DSIY_27   :  Translate StackRelativeIndirectIndexedY,2,7


S_28      :  Translate None,2,8


    .ltorg

@=========================================================================    
@ Opcode Implementations
@=========================================================================    

OpADCDCode:     OpADCD      NA, 0, 0
OpSBCDCode:     OpSBCD      NA, 0, 0


@-------------------------------------------------------------------------
@ optimized functions
@-------------------------------------------------------------------------
LDAA_m0x0:
LDAA_m0x1:
    Translate   Absolute,3,4,0
    OpLDA       M0, 0, 0

LDAA_m1x0:
LDAA_m1x1:
    Translate   Absolute,3,4,0
    OpLDA       M1, 0, 0

LDAD_m0x0:
LDAD_m0x1:
    Translate   DP,2,3,0
    OpLDA       M0, 0, 0

LDAD_m1x0:
LDAD_m1x1:
    Translate   DP,2,3,0
    OpLDA       M1, 0, 0

STAD_m0x0:   
STAD_m0x1:   
    Translate   DP,2,3,0
    OpSTA       M0, 0, 0

STAD_m1x0:   
STAD_m1x1:   
    Translate   DP,2,3,0
    OpSTA       M1, 0, 0

    .ltorg

@-------------------------------------------------------------------------
@ Arithmetic
@-------------------------------------------------------------------------

ADC_m0x1:   ADC_m0x0:   OpADC   M0, 0, 0
ADC_m1x1:   ADC_m1x0:   OpADC   M1, 0, 0

ADC_m0x1I:  ADC_m0x0I:  OpADC   M0IMM, 3, 2
ADC_m1x1I:  ADC_m1x0I:  OpADC   M1IMM, 2, 2

AND_m0x0:   AND_m0x1:   OpAND   M0, 0, 0
AND_m1x0:   AND_m1x1:   OpAND   M1, 0, 0

AND_m0x0I:  AND_m0x1I:  OpAND   M0IMM, 3, 2
AND_m1x0I:  AND_m1x1I:  OpAND   M1IMM, 2, 2

ASLA_m0x0:  ASLA_m0x1:  ASLA_m1x0:  ASLA_m1x1:  OpASLA  0,  1, 2
ASL_m0x0:   ASL_m0x1:   OpASL   M0, 0, 0
ASL_m1x0:   ASL_m1x1:   OpASL   M1, 0, 0

BIT_m0x0:   BIT_m0x1:   OpBIT   M0, 0, 0
BIT_m1x0:   BIT_m1x1:   OpBIT   M1, 0, 0

BIT_m0x0I:  BIT_m0x1I:  OpBIT   M0IMM, 3, 2
BIT_m1x0I:  BIT_m1x1I:  OpBIT   M1IMM, 2, 2

DECA_m0x0:  DECA_m0x1:  OpDEA   M0, 1, 2
DECA_m1x0:  DECA_m1x1:  OpDEA   M1, 1, 2

DEC_m0x0:   DEC_m0x1:   OpDEC   M0, 0, 0
DEC_m1x0:   DEC_m1x1:   OpDEC   M1, 0, 0

DEX_m0x0:   DEX_m1x0:   OpDEX   X0, 1, 2
DEX_m0x1:   DEX_m1x1:   OpDEX   X1, 1, 2

DEY_m0x0:   DEY_m1x0:   OpDEY   X0, 1, 2
DEY_m0x1:   DEY_m1x1:   OpDEY   X1, 1, 2

EOR_m0x0:   EOR_m0x1:   OpEOR   M0, 0, 0
EOR_m1x0:   EOR_m1x1:   OpEOR   M1, 0, 0

EOR_m0x0I:   EOR_m0x1I: OpEOR   M0IMM, 0, 0
EOR_m1x0I:   EOR_m1x1I: OpEOR   M1IMM, 0, 0

INC_m0x0:   INC_m0x1:   OpINC   M0, 0, 0
INC_m1x0:   INC_m1x1:   OpINC   M1, 0, 0

INCA_m0x0:  INCA_m0x1:  OpINA   M0, 1, 2
INCA_m1x0:  INCA_m1x1:  OpINA   M1, 1, 2

INX_m0x0:   INX_m1x0:   OpINX   X0, 1, 2
INX_m0x1:   INX_m1x1:   OpINX   X1, 1, 2

INY_m0x0:   INY_m1x0:   OpINY   X0, 1, 2
INY_m0x1:   INY_m1x1:   OpINY   X1, 1, 2

LSRA_m0x0:  LSRA_m0x1:  OpLSRA  M0, 1, 2
LSRA_m1x0:  LSRA_m1x1:  OpLSRA  M1, 1, 2

LSR_m0x0:   LSR_m0x1:   OpLSR   M0, 0, 0
LSR_m1x0:   LSR_m1x1:   OpLSR   M1, 0, 0

ORA_m0x0:   ORA_m0x1:   OpORA   M0, 0, 0
ORA_m1x0:   ORA_m1x1:   OpORA   M1, 0, 0

ORA_m0x0I:   ORA_m0x1I: OpORA   M0IMM, 0, 0
ORA_m1x0I:   ORA_m1x1I: OpORA   M1IMM, 0, 0

ROLA_m0x0:  ROLA_m0x1:  OpROLA  M0, 1, 2
ROLA_m1x0:  ROLA_m1x1:  OpROLA  M1, 1, 2

ROL_m0x0:   ROL_m0x1:   OpROL   M0, 0, 0
ROL_m1x0:   ROL_m1x1:   OpROL   M1, 0, 0

RORA_m0x0:  RORA_m0x1:  OpRORA  M0, 1, 2
RORA_m1x0:  RORA_m1x1:  OpRORA  M1, 1, 2

ROR_m0x0:   ROR_m0x1:   OpROR   M0, 0, 0
ROR_m1x0:   ROR_m1x1:   OpROR   M1, 0, 0

SBC_m0x0:   SBC_m0x1:   OpSBC   M0, 0, 0
SBC_m1x0:   SBC_m1x1:   OpSBC   M1, 0, 0

SBC_m0x0I:  SBC_m0x1I:  OpSBC   M0IMM, 0, 0
SBC_m1x0I:  SBC_m1x1I:  OpSBC   M1IMM, 0, 0

TRB_m0x0:   TRB_m0x1:   OpTRB   M0, 0, 0
TRB_m1x0:   TRB_m1x1:   OpTRB   M1, 0, 0

TSB_m0x0:   TSB_m0x1:   OpTSB   M0, 0, 0
TSB_m1x0:   TSB_m1x1:   OpTSB   M1, 0, 0
    .ltorg

@-------------------------------------------------------------------------
@ Compare
@-------------------------------------------------------------------------

CMP_m0x0:   CMP_m0x1:   OpCMP   M0, 0, 0 
CMP_m1x0:   CMP_m1x1:   OpCMP   M1, 0, 0 

CPX_m0x0:   CPX_m1x0:   OpCPX   X0, 0, 0 
CPX_m0x1:   CPX_m1x1:   OpCPX   X1, 0, 0 

CPY_m0x0:   CPY_m1x0:   OpCPY   X0, 0, 0 
CPY_m0x1:   CPY_m1x1:   OpCPY   X1, 0, 0 

CMP_m0x0I:   CMP_m0x1I:  OpCMP   M0IMM, 3, 2 
CMP_m1x0I:   CMP_m1x1I:  OpCMP   M1IMM, 2, 2

CPX_m0x0I:   CPX_m1x0I:  OpCPX   X0IMM, 3, 2 
CPX_m0x1I:   CPX_m1x1I:  OpCPX   X1IMM, 2, 2 

CPY_m0x0I:   CPY_m1x0I:  OpCPY   X0IMM, 3, 2 
CPY_m0x1I:   CPY_m1x1I:  OpCPY   X1IMM, 2, 2 

@-------------------------------------------------------------------------
@ Register transfer
@-------------------------------------------------------------------------

TAX_m0x0:   OpTAX   M0X0, 1, 2
TAX_m0x1:   OpTAX   M0X1, 1, 2
TAX_m1x0:   OpTAX   M1X0, 1, 2
TAX_m1x1:   OpTAX   M1X1, 1, 2

TAY_m0x0:   OpTAY   M0X0, 1, 2
TAY_m0x1:   OpTAY   M0X1, 1, 2
TAY_m1x0:   OpTAY   M1X0, 1, 2
TAY_m1x1:   OpTAY   M1X1, 1, 2

TXA_m0x0:   OpTXA   M0X0, 1, 2
TXA_m0x1:   OpTXA   M0X1, 1, 2
TXA_m1x0:   OpTXA   M1X0, 1, 2
TXA_m1x1:   OpTXA   M1X1, 1, 2

TYA_m0x0:   OpTYA   M0X0, 1, 2
TYA_m0x1:   OpTYA   M0X1, 1, 2
TYA_m1x0:   OpTYA   M1X0, 1, 2
TYA_m1x1:   OpTYA   M1X1, 1, 2

TSX_m0x0:   TSX_m0x1:   TSX_m1x0:   TSX_m1x1:   OpTSX   NA, 1, 2 
TXS_m0x0:   TXS_m0x1:   TXS_m1x0:   TXS_m1x1:   OpTXS   NA, 1, 2 

TXY_m0x0:   TXY_m1x0:   OpTXY   X0, 1, 2
TYX_m0x0:   TYX_m1x0:   OpTYX   X0, 1, 2
TXY_m0x1:   TXY_m1x1:   OpTXY   X1, 1, 2
TYX_m0x1:   TYX_m1x1:   OpTYX   X1, 1, 2

TCD_m0x0:   TCD_m0x1:   OpTCD   M0, 1, 2
TCD_m1x0:   TCD_m1x1:   OpTCD   M1, 1, 2

TDC_m0x0:   TDC_m0x1:   OpTDC   M0, 1, 2
TDC_m1x0:   TDC_m1x1:   OpTDC   M1, 1, 2

TCS_m0x0:   TCS_m0x1:   OpTCS   M0, 1, 2
TCS_m1x0:   TCS_m1x1:   OpTCS   M1, 1, 2

TSC_m0x0:   TSC_m0x1:   OpTSC   M0, 1, 2
TSC_m1x0:   TSC_m1x1:   OpTSC   M1, 1, 2

XBA_m0x0:   XBA_m0x1:   OpXBA   M0, 1, 3
XBA_m1x0:   XBA_m1x1:   OpXBA   M1, 1, 3

.GLOBAL SnesB
SnesB:      .long   0

@=========================================================================
@ Memory store/load
@=========================================================================

@ LDA/X/Y
@
LDA_m0x0:   LDA_m0x1:   OpLDA   M0, 0, 0
LDA_m1x0:   LDA_m1x1:   OpLDA   M1, 0, 0

LDX_m0x0:   LDX_m1x0:   OpLDX   X0, 0, 0
LDX_m0x1:   LDX_m1x1:   OpLDX   X1, 0, 0
LDY_m0x0:   LDY_m1x0:   OpLDY   X0, 0, 0
LDY_m0x1:   LDY_m1x1:   OpLDY   X1, 0, 0


@ immediate modes for LDA/X/Y
@
LDA_m0x0I:  LDA_m0x1I:  OpLDA   M0IMM, 3, 2
LDA_m1x0I:  LDA_m1x1I:  OpLDA   M1IMM, 2, 2

LDX_m0x0I:  LDX_m1x0I:  OpLDX   X0IMM, 3, 2
LDX_m0x1I:  LDX_m1x1I:  OpLDX   X1IMM, 2, 2
LDY_m0x0I:  LDY_m1x0I:  OpLDY   X0IMM, 3, 2
LDY_m0x1I:  LDY_m1x1I:  OpLDY   X1IMM, 2, 2

@ STA/X/Y/Z
@
STA_m0x0:   STA_m0x1:   OpSTA   M0, 0, 0
STA_m1x0:   STA_m1x1:   OpSTA   M1, 0, 0

STX_m0x0:   STX_m1x0:   OpSTX   X0, 0, 0
STX_m0x1:   STX_m1x1:   OpSTX   X1, 0, 0

STY_m0x0:   STY_m1x0:   OpSTY   X0, 0, 0
STY_m0x1:   STY_m1x1:   OpSTY   X1, 0, 0

STZ_m0x0:   STZ_m0x1:   OpSTZ   M0, 0, 0
STZ_m1x0:   STZ_m1x1:   OpSTZ   M1, 0, 0


MVP_m0x0:   OpMVP   M0X0, 0, 0
MVP_m0x1:   OpMVP   M0X1, 0, 0
MVP_m1x0:   OpMVP   M1X0, 0, 0
MVP_m1x1:   OpMVP   M1X1, 0, 0

MVN_m0x0:   OpMVN   M0X0, 0, 0
MVN_m0x1:   OpMVN   M0X1, 0, 0
MVN_m1x0:   OpMVN   M1X0, 0, 0
MVN_m1x1:   OpMVN   M1X1, 0, 0

@------------------------------------------------------
@ MVP/MVN
@------------------------------------------------------
OpMVP_Code:
    stmfd   sp!, {r3, r6, lr}
    add     SnesA, SnesA, #0x00010000
    mov     r6, #32
9:
    @ version 0.27 fix
    ldrb    r0, [SnesPC, #2]        @ source
    @ version 0.27 fix end
    
    add     r0, SnesY, r0, lsl #16
    SetRead
    TranslateAddress    0
    ReadData8
    mov     r3, r1
    
    @ version 0.27 fix
    ldrb    r0, [SnesPC, #1]        @ dest
    @ version 0.27 fix end
    
    add     r0, SnesX, r0, lsl #16
    SetWrite
    TranslateAddress    0
    mov     r1, r3
    WriteData8

    @ version 0.27 fix
    sub     SnesX, SnesX, #1
    sub     SnesY, SnesY, #1
    @ version 0.27 fix end
    
    add     SnesCYCLES, SnesCYCLES, #(7 << CYCLE_SHIFT)
    bic     SnesX, SnesX, r7                @ r7 = #0x00ff0000 or #0x0000ff00 depending on the xBit
    bic     SnesY, SnesY, r7
    subs    SnesA, SnesA, #0x00010000
    beq     OpMVP_Code_End
    subs    r6, r6, #1
    bne     9b

OpMVP_Code_End:
    tsts    SnesA, SnesA
    sub     SnesA, SnesA, #0x00010000       @ make SnesA = 0xffff

    ldrb    r0, [SnesPC, #1]
@    mov		r1, SnesDBR
    bic     SnesDBR, SnesDBR, #0xff
    orr     SnesDBR, SnesDBR, r0
    addeq   SnesPC, SnesPC, #3
    /*cmp		r1, SnesDBR						@ archeide
    beq		3f

    CacheMemoryMap
3:*/
    ldmfd   sp!, {r3, r6, lr}
    bx      lr

@------------------------------------------------------
@ MVP/MVN
@------------------------------------------------------
OpMVN_Code:
    stmfd   sp!, {r3, r6, lr}
    add     SnesA, SnesA, #0x00010000
    mov     r6, #32
9:
    ldrb    r0, [SnesPC, #2]        @ source
    add     r0, SnesX, r0, lsl #16
    SetRead
    TranslateAddress    0
    ReadData8
    mov     r3, r1
    ldrb    r0, [SnesPC, #1]        @ dest
    add     r0, SnesY, r0, lsl #16
    SetWrite
    TranslateAddress    0
    mov     r1, r3
    WriteData8

    add     SnesX, SnesX, #1
    add     SnesY, SnesY, #1
    add     SnesCYCLES, SnesCYCLES, #(7 << CYCLE_SHIFT)
    bic     SnesX, SnesX, r7                @ r7 = #0x00ff0000 or #0x0000ff00 depending on the xBit
    bic     SnesY, SnesY, r7
    subs    SnesA, SnesA, #0x00010000
    beq     OpMVN_Code_End
    subs    r6, r6, #1
    bne     9b
    
OpMVN_Code_End:
    tsts    SnesA, SnesA
    sub     SnesA, SnesA, #0x00010000       @ make SnesA = 0xffff

    ldrb    r0, [SnesPC, #1]
@    mov		r1, SnesDBR 					@ archeide
    bic     SnesDBR, SnesDBR, #0xff
    orr     SnesDBR, SnesDBR, r0
    addeq   SnesPC, SnesPC, #3
    /*cmp		r1, SnesDBR						@ archeide
    beq		3f

    CacheMemoryMap
3:*/
    ldmfd   sp!, {r3, r6, lr}
    bx      lr

    .ltorg

@-------------------------------------------------------------------------
@ Flags manipulation
@-------------------------------------------------------------------------

CLC_m0x0:   CLC_m0x1:   CLC_m1x0:   CLC_m1x1:   OpCLC   NA, 1, 2
CLD_m0x0:   CLD_m0x1:   CLD_m1x0:   CLD_m1x1:   OpCLD   NA, 1, 2
CLI_m0x0:   CLI_m0x1:   CLI_m1x0:   CLI_m1x1:   OpCLI   NA, 1, 2
CLV_m0x0:   CLV_m0x1:   CLV_m1x0:   CLV_m1x1:   OpCLV   NA, 1, 2
SEC_m0x0:   SEC_m0x1:   SEC_m1x0:   SEC_m1x1:   OpSEC   NA, 1, 2
SED_m0x0:   SED_m0x1:   SED_m1x0:   SED_m1x1:   OpSED   NA, 1, 2
SEI_m0x0:   SEI_m0x1:   SEI_m1x0:   SEI_m1x1:   OpSEI   NA, 1, 2
SEV_m0x0:   SEV_m0x1:   SEV_m1x0:   SEV_m1x1:   OpSEV   NA, 1, 2
XCE_m0x0:   XCE_m0x1:   XCE_m1x0:   XCE_m1x1:   OpXCE   NA, 1, 2

SEP_m0x0:   SEP_m0x1:   SEP_m1x0:   SEP_m1x1:   OpSEP   NA, 2, 3
REP_m0x0:   REP_m0x1:   REP_m1x0:   REP_m1x1:   OpREP   NA, 2, 3

    .ltorg


@-------------------------------------------------------------------------
@ Stack Opcodes
@-------------------------------------------------------------------------

PEA_m0x0:   PEA_m0x1:   PEA_m1x0:   PEA_m1x1:   OpPEA   NA, 3, 5
PEI_m0x0:   PEI_m0x1:   PEI_m1x0:   PEI_m1x1:   OpPEI   NA, 2, 6
PER_m0x0:   PER_m0x1:   PER_m1x0:   PER_m1x1:   OpPER   NA, 3, 6

PHA_m0x0:   PHA_m0x1:   OpPHA   M0, 1, 3
PHA_m1x0:   PHA_m1x1:   OpPHA   M1, 1, 3
PLA_m0x0:   PLA_m0x1:   OpPLA   M0, 1, 4
PLA_m1x0:   PLA_m1x1:   OpPLA   M1, 1, 4

PHX_m0x0:   PHX_m1x0:   OpPHX   X0, 1, 3
PHX_m0x1:   PHX_m1x1:   OpPHX   X1, 1, 3
PHY_m0x0:   PHY_m1x0:   OpPHY   X0, 1, 3
PHY_m0x1:   PHY_m1x1:   OpPHY   X1, 1, 3

PLX_m0x0:   PLX_m1x0:   OpPLX   X0, 1, 4
PLX_m0x1:   PLX_m1x1:   OpPLX   X1, 1, 4
PLY_m0x0:   PLY_m1x0:   OpPLY   X0, 1, 4
PLY_m0x1:   PLY_m1x1:   OpPLY   X1, 1, 4 

PHB_m0x0:   PHB_m0x1:   PHB_m1x0:   PHB_m1x1:   OpPHB   NA, 1, 3
PHD_m0x0:   PHD_m0x1:   PHD_m1x0:   PHD_m1x1:   OpPHD   NA, 1, 4
PHK_m0x0:   PHK_m0x1:   PHK_m1x0:   PHK_m1x1:   OpPHK   NA, 1, 3
PHP_m0x0:   PHP_m0x1:   PHP_m1x0:   PHP_m1x1:   OpPHP   NA, 1, 3
PLB_m0x0:   PLB_m0x1:   PLB_m1x0:   PLB_m1x1:   OpPLB   NA, 1, 4
PLD_m0x0:   PLD_m0x1:   PLD_m1x0:   PLD_m1x1:   OpPLD   NA, 1, 5
PLP_m0x0:   PLP_m0x1:   PLP_m1x0:   PLP_m1x1:   OpPLP   NA, 1, 4

@-------------------------------------------------------------------------
@ Branch Opcodes
@-------------------------------------------------------------------------

RTI_m0x0:   RTI_m0x1:   RTI_m1x0:   RTI_m1x1:   OpRTI   NA, 0, 7
RTL_m0x0:   RTL_m0x1:   RTL_m1x0:   RTL_m1x1:   OpRTL   NA, 0, 6
RTS_m0x0:   RTS_m0x1:   RTS_m1x0:   RTS_m1x1:   OpRTS   NA, 0, 6

BRL_m0x0:   BRL_m0x1:   BRL_m1x0:   BRL_m1x1:   OpBRL   NA, 3, 3

.GLOBAL CPU_LoopSpeedHacks
CPU_LoopSpeedHacks:       .word	0
.GLOBAL CPU_NextCycles
CPU_NextCycles:			  .word	0
.GLOBAL CPU_WaitAddress
CPU_WaitAddress:		  .word	0
.GLOBAL CPU_LoopAddress
CPU_LoopAddress:		  .word	0

    .ltorg

BRA_m0x0:   BRA_m0x1:   BRA_m1x0:   BRA_m1x1:   OpBRA   NA, 2, 2
BCC_m0x0:   BCC_m0x1:   BCC_m1x0:   BCC_m1x1:   OpBR    SnesFlagC, 0, 2, 2
BCS_m0x0:   BCS_m0x1:   BCS_m1x0:   BCS_m1x1:   OpBR    SnesFlagC, 1, 2, 2
BVC_m0x0:   BVC_m0x1:   BVC_m1x0:   BVC_m1x1:   OpBR    SnesFlagV, 0, 2, 2
BVS_m0x0:   BVS_m0x1:   BVS_m1x0:   BVS_m1x1:   OpBR    SnesFlagV, 1, 2, 2
BNE_m0x0:   BNE_m0x1:   BNE_m1x0:   BNE_m1x1:   OpBR    SnesFlagZ, 0, 2, 2
BEQ_m0x0:   BEQ_m0x1:   BEQ_m1x0:   BEQ_m1x1:   OpBR    SnesFlagZ, 1, 2, 2
BPL_m0x0:   BPL_m0x1:   BPL_m1x0:   BPL_m1x1:   OpBR    SnesFlagN, 0, 2, 2
BMI_m0x0:   BMI_m0x1:   BMI_m1x0:   BMI_m1x1:   OpBR    SnesFlagN, 1, 2, 2

JMP_m0x0:   JMP_m0x1:   JMP_m1x0:   JMP_m1x1:   OpJMP   NA, 0, 0
JML_m0x0:   JML_m0x1:   JML_m1x0:   JML_m1x1:   OpJMP   NA, 0, 0

JSL_m0x0:   JSL_m0x1:   JSL_m1x0:   JSL_m1x1:   
    @ version 0.25 optimization
    @
    Translate AbsoluteLongPC, 0, 0, 0
    OpJSR   AbsoluteLongPC, 4, 8

JSR_m0x0A:   JSR_m0x1A:   JSR_m1x0A:   JSR_m1x1A:   
    @ version 0.25 optimization
    @
    Translate AbsolutePC, 0, 0, 0 
    OpJSR   NA, 3, 6

JSR_m0x0I:   JSR_m0x1I:   JSR_m1x0I:   JSR_m1x1I:   
    @ version 0.25 optimization
    @
    Translate AbsoluteIndexedXIndirectPC, 0, 0, 0 
    OpJSR   NA, 3, 6

@@A_J36     :  Translate AbsolutePC,0,6

.GLOBAL SnesPCOffset
SnesPCOffset:       .long   0

    .ltorg

@-------------------------------------------------------------------------
@ Others
@-------------------------------------------------------------------------

COP_m0x0:   COP_m0x1:   COP_m1x0:   COP_m1x1:   OpCOP   NA, 0, 0
WAI_m0x0:   WAI_m0x1:   WAI_m1x0:   WAI_m1x1:   OpWAI   NA, 0, 0
BRK_m0x0:   BRK_m0x1:   BRK_m1x0:   BRK_m1x1:   OpBRK   NA, 0, 0

@-------------------------------------------------------------------------
@ specially added instructions
@-------------------------------------------------------------------------

STP_m0x0:   STP_m0x1:   STP_m1x0:   STP_m1x1:   OpSTP   NA, 0, 0
RES_m0x0:   RES_m0x1:   RES_m1x0:   RES_m1x1:   OpRES   NA, 0, 0
NOP_m0x0:   NOP_m0x1:   NOP_m1x0:   NOP_m1x1:   OpNOP   NA, 1, 2

/* ------------------------- IO wrappers for SNEmulDS ----------------- */

MemReloadPC:
	ldr		r2, =SNEmulRegisters
	stmia	r2,	{r3-r12,lr}			@Save Registers

	bic		r1, r0, #0xFF000000 	@ real address
	mov     r0, r1, lsr #13			@ Get Memory block
	
	stmfd	sp!, {r1, r2}
	bl		mem_checkReload
	ldmfd	sp!, {r1, r2}
	add		r0, r0, r1 				@ get effective address
	
	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12,lr}			@ Load Registers
	bx		lr   

MemReload2:
	ldr		r2, =SNEmulRegisters
	stmia	r2,	{r3-r12,lr}			@Save Registers

	bic		r1, r0, #0xFF000000 	@ real address
	mov     r0, r1, lsr #13			@ Get Memory block
	stmfd	sp!, {r1, r2}
	bl		mem_checkReload
	ldmfd	sp!, {r1, r2}
	add		r0, r0, r1 				@ get effective address
	
	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12,lr}			@ Load Registers
	bx		lr

/* archeide: we are here if the program acceded a region of ROM
			  not loaded */	
MemReload:
  	mov     r0, r1, lsr #13	@ Get Memory block
  	
  	stmfd	sp!, {r1, r2, lr}
  	@mov		r4, r1			@ r4 will be saved by C function
	bl		mem_checkReload
	ldmfd	sp!, {r1, r2, lr}
	add		r0, r0, r1 		@ get effective address
	mov		r1, r2	

	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12,lr}	@ Load Registers
	
	movs	r2, #0	@ clear ne flag, can we avoid this ?
	bx		lr
	

IORead8:
	ldr		r2, =SNEmulRegisters
	stmia	r2,	{r3-r12,lr}	@Save Registers

	bic		r1, r0,  #0xFF000000 @ real address
	and		r0, r0,  #0xFF000000 @ memory type
	
	cmp		r0, #0x80000000
	beq		MemReload

	bl		IO_getbyte 				@ SNEmul entry point

	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12,lr}	@ Load Registers

	mov		r1, r0	@result
	movs	r2, #1 		@ set ne flag, can we avoid this ?
	bx		lr

IOWrite8:
	ldr		r2, =SNEmulRegisters
	stmia	r2,	{r3-r12,lr}	@Save Registers

/*	ldr		r2, =0x81002100
	cmp		r0, r2
	bne		1f
	
	bic		r0, r0,  #0xFF000000 @ real address
	blx		PPU_port_write	

	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12,lr}	@ Load Registers
	bx		lr
1:	*/
	mov		r2, r1
	bic		r1, r0,  #0xFF000000 @ real address
	and		r0, r0,  #0xFF000000 @ memory type

/*
	cmp		r0, #0x82000000 @ Is CPU/DMA I/O ?
	bne		1f
	
	ldr		r3, =IOWrite_DMA
	ldr		r3, [r3, r1]
	blx		r3

	b		2f
1:
	cmp		r0, #0x81000000	@ Is PPU I/O ?
	bne		1f
	
	ldr		r3, =IOWrite_PPU
	sub		r0, r1, #0x2100	
	mov		r1, r2
	ldr		r3, [r3, r0, lsl #2]
	
	b		2f	
1:
*/	
	cmp		r0, #0x80000000
	beq		MemReload


	
	bl		IO_setbyte 				@ SNEmul entry point
2:
	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12,lr}	@ Load Registers
	movs	r2, #1 		@ set ne flag, can we avoid this ?
	bx		lr


IORead16:
	ldr		r2, =SNEmulRegisters
	stmia	r2,	{r3-r12,lr}	@Save Registers

	bic		r1, r0,  #0xFF000000 @ real address
	and		r0, r0,  #0xFF000000 @ memory type

	cmp		r0, #0x80000000
	beq		MemReload

	bl		IO_getword 				@ SNEmul entry point
	
	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12,lr}	@ Load Registers

	mov		r1, r0	@result
	movs	r2, #1 		@ set ne flag, can we avoid this ?
	bx		lr

IOWrite16:
	ldr		r2, =SNEmulRegisters
	stmia	r2,	{r3-r12,lr}	@Save Registers

	mov		r2, r1
	bic		r1, r0,  #0xFF000000 @ real address
	and		r0, r0,  #0xFF000000 @ memory type

	cmp		r0, #0x80000000
	beq		MemReload

	bl		IO_setword 				@ SNEmul entry point
	
	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12,lr}	@ Load Registers
	
	movs	r2, #1 		@ set ne flag, can we avoid this ?
	bx		lr


@=========================================================================
@ CPU loop for SNEmulDS
@=========================================================================

    
.GLOBAL CPU_update
CPU_update:
	stmfd	sp!, {r0-r12}

	@ Need to load some SNES registers 
	ldr		r1,	=SNEmulRegisters
	ldmia	r1,	{r3-r12}

	and		r1, SnesMXDI, #0x000000FF
	mov		r1, r1, lsr #2 
	SetMXDIDecoder
	str		EmuDecoder, SaveEmuDecoder

	@  DP Cache
    ldr     r1, =DPCache
    ldr     r2, =MemoryMap
    mov     r3, #0x10
DPCacheLoop:
    ldr     r0, [r2], #4
	str     r0, [r1], #4
	subs    r3, r3, #1
	bne     DPCacheLoop	

	CacheMemoryMap	
	    
/*	ldr		r0, =TimeOutHandler
	ldr		r1, =TimeOutHandlerAddress
	str		r0, [r1]*/
    
/*	ldr		r1,	=SNEmulRegisters
	stmia	r1,	{r3-r12}*/    
    
    ldmfd	sp!, {r0-r12}
    bx		lr    
    
.GLOBAL CPU_goto2
CPU_goto2:
	stmfd	sp!, {r1-r12, lr}

	@ here put SNES regs in DS regs

	ldr		r1,	=SNEmulRegisters
	ldmia	r1,	{r3-r12}

	@ r0 : Number of cycles to execute
    mov     SnesCYCLES, SnesCYCLES, lsl #(32-CYCLE_SHIFT)
    mov     SnesCYCLES, SnesCYCLES, lsr #(32-CYCLE_SHIFT)
    
	subs    SnesCYCLES, SnesCYCLES, r0, lsl #CYCLE_SHIFT		

Fetch:
    .ifeq   debug-1

	ldr		r0, CPU_log
	cmp		r0, #1
@	bne		SkipDebug
	bne		FastDebug
  
  	ldr		r2, =SNEmulRegisters
	stmia	r2,	{r3-r12}			@Save Registers  
  	bl		trace_CPU
	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12}			@Load Registers
	b		SkipDebug    

FastDebug:
  	ldr		r2, =SNEmulRegisters
	stmia	r2,	{r3-r12}			@Save Registers  
  	bl		trace_CPUFast
	ldr		r2, =SNEmulRegisters
	ldmia	r2,	{r3-r12}			@Load Registers    


SkipDebug:
	subs    SnesCYCLES, SnesCYCLES, #0

	.endif

	OpcodeFetch
	
EndOfCPULoop:

	ldr		r0,	=SNEmulRegisters
	stmia	r0,	{r3-r12}
				
	ldmfd	sp!, {r1-r12,lr}
	bx		lr

.GLOBAL CPU_init
CPU_init:
	stmfd	sp!, {r3-r12, lr}

   	ldr     SnesSP, =0x01FF0000         @ SP = ???, PBR = 0
    mov     SnesD, #0                   @ D = 0, DBR = 0
    mov     SnesX, #0
    mov     SnesY, #0
    mov     SnesA, #0
    mov     SnesCV, #0
    mov     SnesNZ, #1
    mov     SnesMXDI, #0
    mov     SnesCYCLES, #0

	ldr		SnesMemMap, =MemoryMap    
@    ldr     EmuDecoder, =m1x1Decoder

    @ set emulation bit as on; m=1, x=1, d=0, i=1, cycles=0, e=1, romSpeed = 8 (slowROM)
    orr     SnesMXDI, SnesMXDI, #(SnesFlagE+SnesFlagM+SnesFlagX+SnesFlagI)      
 
	str		SnesA, A
	str		SnesX, X
	str		SnesY, Y
	str		SnesPC, PCptr
	str		SnesMXDI, SaveR8	@ MXDI CV CYCES WAI
	str		SnesSP, S	@ S and PB
	str		SnesNZ, SaveR6	 @ RV
	str		SnesD, D	@ D and DB	
	str		SnesMemMap,	SaveMemoryMap
	str		EmuDecoder, SaveEmuDecoder  

/*
#define MAP_RELOAD      0x80000000
#define MAP_PPU         0x81000000
#define MAP_CPU         0x82000000
#define MAP_DSP         0x83000000
#define MAP_LOROM_SRAM  0x84000000
#define MAP_HIROM_SRAM  0x85000000
#define MAP_NONE        0x86000000
#define MAP_LAST        0x8F000000
*/
/*	ldr		r0, =MemFunctions
	ldr		r1, =Read8Reload
	str		r1, [r0, #0]	
	ldr		r1, =MemPPU
	str		r1, [r0, #0]
	ldr		r1, =MAP_CPU
	str		r1, [r0, #0]	
	ldr		r1, =
	str		r1, [r0, #0]	
	ldr		r1, =MAP_
	str		r1, [r0, #0]*/	
		


	bl		CPU_update
    
	ldr		r0,	=CPU_log
	mov		r1, #0
	str		r1, [r0]	  
    
    ldmfd	sp!, {r3-r12, lr}
    bx		lr  


.ltorg

SNEmulRegisters:

SaveEmuDecoder:	.word	0 	@ r3
SaveMemoryMap:	.word	0	@ r4
.GLOBAL D
D:				.word	0	@ r5
.GLOBAL SaveR6
SaveR6:			.word	0	@ r6
.GLOBAL S
S:				.word 	0	@ r7
.GLOBAL SaveR8
SaveR8:			.word	0	@ r8
.GLOBAL PCptr
PCptr:			.word	0	@ r9
.GLOBAL X
X:				.word	0	@ r10
.GLOBAL Y
Y:				.word	0	@ r11
.GLOBAL A
A:				.word	0	@ r12
savelr:			.word 	0	@ r14	


.GLOBAL COPaddress
COPaddress:         .word   0
.GLOBAL BRKaddress
BRKaddress:         .word   0

.GLOBAL CPU_log
CPU_log:		.word	0

.GLOBAL PC
PC:	.word	0
.GLOBAL PB
PB:	.word	0
.GLOBAL DB
DB:	.word	0
.GLOBAL P
P:	.word 	0
.GLOBAL Cycles
Cycles:	.word	0

.GLOBAL AsmDebug
AsmDebug:	
AsmDebug1:
		.word	0
		.word	0
		.word	0
		.word	0
AsmDebug2:		
		.word	0
		.word	0
		.word	0
		.word	0
AsmDebug3:		
		.word	0
		.word	0
		.word	0
		.word	0
AsmDebug4:		
		.word	0
		.word	0
		.word	0
		.word	0
	

@	.include	"../opc_misc.s"
