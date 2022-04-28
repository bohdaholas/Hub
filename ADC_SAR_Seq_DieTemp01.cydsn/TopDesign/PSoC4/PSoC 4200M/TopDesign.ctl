--=============================================================================
--The following directives assign pins to the locations specific for the
--CY8CKIT-043 and CY8CKIT-044 kit.
--=============================================================================

attribute port_location of Pin_Vin0(0) : label is "PORT(2,0)";      --ADC input 0
attribute port_location of Pin_Vin1(0) : label is "PORT(2,1)";      --ADC input 1
attribute port_location of \UART:tx(0)\ : label is "PORT(7,1)";     --UART Tx pin