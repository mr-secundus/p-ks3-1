// events_app.h
                                         

//-----------------------------------------------------------------------------

const short msgCmd_ADC_Start			= 72;			// команда пуска измерения
const short msgCmd_ADC_Stop				= 73;			// команда останова измерения
const short msgADCStart						= 74;			// уведомление о выполнении пуска измерения
const short msgADCStop						= 75;			// уведомление о выполнении останова измерения 
const short msgAI4R_CmdConfig			= 76;			// конфигурирование АЦП1
const short msgAI4R_CmdStart			= 77;			// пуск АЦП1
const short msgAI4R_CmdStop				= 78;			// останов АЦП1
const short msgAI4R_State					= 79;			// состояние задачи управления AI4R
																						// Param1 - состояние
const short msgAI4R_ConfigOk			= 80;
const short msgAI4R_StartOk				= 81;
const short msgAI4R_StopOk				= 82;

const short msgSt_ADC1_Config			= 83;
const short msgSt_ADC1_Start			= 84;
const short msgSt_ADC1_Meas				= 85;
const short msgSt_ADC1_Stop				= 86;
const short msgSt_ADC2_Config			= 87;
const short msgSt_ADC2_Start			= 88;
const short msgSt_ADC2_Meas				= 89;
const short msgSt_ADC2_Stop				= 90;

