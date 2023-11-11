import serial
import struct
import os
import sys
import glob
from time import sleep

''' Bootloader Commands '''
CBL_GET_VER_CMD              = 0x10
CBL_GET_HELP_CMD             = 0x11
CBL_GET_CID_CMD              = 0x12
CBL_GET_RDP_STATUS_CMD       = 0x13
CBL_GO_TO_ADDR_CMD           = 0x14
CBL_FLASH_ERASE_CMD          = 0x15
CBL_MEM_WRITE_CMD            = 0x16

INVALID_SECTOR_NUMBER        = 0x00
VALID_SECTOR_NUMBER          = 0x01
UNSUCCESSFUL_ERASE           = 0x02
SUCCESSFUL_ERASE             = 0x03

FLASH_PAYLOAD_WRITE_FAILED   = 0x00
FLASH_PAYLOAD_WRITE_PASSED   = 0x01

verbose_mode = 1
Memory_Write_Active = 0

def Check_Serial_Ports():
    Serial_Ports = []
    
    if sys.platform.startswith('win'):
        Ports = ['COM%s' % (i + 1) for i in range(256)]
    else:
        raise EnvironmentError("Error !! Unsupported Platform \n")
    
    for Serial_Port in Ports:
        try:
            test = serial.Serial(Serial_Port)
            test.close()
            Serial_Ports.append(Serial_Port)
        except (OSError, serial.SerialException):
            pass
    
    return Serial_Ports

def Serial_Port_Configuration(Port_Number):
    global Serial_Port_Obj
    try:
        Serial_Port_Obj = serial.Serial(Port_Number, 115200, timeout = 2)
    except:
        print("\nError !! That was not a valid port")
    
        Port_Number = Check_Serial_Ports()
        if(not Port_Number):
            print("\nError !! No ports Detected")
        else:
            print("\nHere are some available ports on your PC. Try Again !!")
            print("\n   ", Port_Number)
        return -1
    
    if Serial_Port_Obj.is_open:
        print("Port Open Success \n")
    else:
        print("Port Open Failed \n")

def Write_Data_To_Serial_Port(Value, Length):
    _data = struct.pack('>B', Value)
    if(verbose_mode):
        Value = bytearray(_data)
        print("   "+"0x{:02x}".format(Value[0]), end = ' ')
        if(Memory_Write_Active and (not verbose_mode)):
            print("#", end = ' ')
        Serial_Port_Obj.write(_data)

def Read_Serial_Port(Data_Len):
    
    Serial_Value = Serial_Port_Obj.read(Data_Len)
    Serial_Value_len = len(Serial_Value)
    while Serial_Value_len <= 0:
        Serial_Value = Serial_Port_Obj.read(Data_Len)
        Serial_Value_len = len(Serial_Value)
        print("Waiting Replay from the BootLoader")
    return Serial_Value
    
    '''
    Serial_Value = Serial_Port_Obj.read(Data_Len)
    return Serial_Value
    '''

def Read_Data_From_Serial_Port(Command_Code):
    Length_To_Follow = 0
    
    BL_ACK = Read_Serial_Port(2)
    if(len(BL_ACK)):
        BL_ACK_Array = bytearray(BL_ACK)
        if(BL_ACK_Array[0] == 0xCD):
            print ("\n   Received Acknowledgement from Bootloader")
            Length_To_Follow = BL_ACK_Array[1]
            print("   Preparing to receive (", int(Length_To_Follow), ") bytes from the bootloader")
            if(Command_Code == CBL_GET_VER_CMD):
                Process_CBL_GET_VER_CMD(Length_To_Follow)
            elif (Command_Code == CBL_GET_HELP_CMD):
                Process_CBL_GET_HELP_CMD(Length_To_Follow)
            elif (Command_Code == CBL_GET_CID_CMD):
                Process_CBL_GET_CID_CMD(Length_To_Follow)
            elif (Command_Code == CBL_GET_RDP_STATUS_CMD):
                Process_CBL_GET_RDP_STATUS_CMD(Length_To_Follow)
            elif (Command_Code == CBL_GO_TO_ADDR_CMD):
                Process_CBL_GO_TO_ADDR_CMD(Length_To_Follow)
            elif (Command_Code == CBL_FLASH_ERASE_CMD):
                Process_CBL_FLASH_ERASE_CMD(Length_To_Follow)
            elif (Command_Code == CBL_MEM_WRITE_CMD):
                Process_CBL_MEM_WRITE_CMD(Length_To_Follow)
        else:
            print ("\n   Received Not-Acknowledgement from Bootloader")
            sys.exit()
        
def Process_CBL_GET_VER_CMD(Data_Len):
    Serial_Data = Read_Serial_Port(Data_Len)
    _value_ = bytearray(Serial_Data)
    print("\n   Bootloader Vendor ID : ", _value_[0])
    print("   Bootloader Version   : ", _value_[1], ".", _value_[2], ".", _value_[3])

def Process_CBL_GET_HELP_CMD(Data_Len):
    Serial_Data = Read_Serial_Port(Data_Len)
    _value_ = bytearray(Serial_Data)
    print("\n   Supported Commands : ", end = ' ')
    for command in _value_:
        print(hex(command), end = ' ')

def Process_CBL_GET_CID_CMD(Data_Len):
    Serial_Data = Read_Serial_Port(Data_Len)
    CID = (Serial_Data[1] << 8) | Serial_Data[0]
    print("\n   Chip Identification Number : ", hex(CID))

def Process_CBL_GET_RDP_STATUS_CMD(Data_Len):
    Serial_Data = Read_Serial_Port(Data_Len)
    _value_ = bytearray(Serial_Data)
    if(_value_[0] == 0xEE):
        print("\n   Error While Reading FLASH Protection level !!")
    elif(_value_[0] == 0xA5):
        print("\n   FLASH Protection : LEVEL 0")
    elif(_value_[0] == 0x00):
        print("\n   FLASH Protection : LEVEL 1")
    elif(_value_[0] == 0xCC):
        print("\n   FLASH Protection : LEVEL 2")

def Process_CBL_GO_TO_ADDR_CMD(Data_Len):
    Serial_Data = Read_Serial_Port(Data_Len)
    _value_ = bytearray(Serial_Data)
    if(_value_[0] == 1):
        print("\n   Address Status is Valid")
    else:
        print("\n   Address Status is InValid")

def Process_CBL_FLASH_ERASE_CMD(Data_Len):
    BL_Erase_Status = 0
    Serial_Data = Read_Serial_Port(Data_Len)
    if(len(Serial_Data)):
        BL_Erase_Status = bytearray(Serial_Data)
        if(BL_Erase_Status[0] == INVALID_SECTOR_NUMBER):
            print("\n   Erase Status -> Invalid Sector Number ")
        elif (BL_Erase_Status[0] == UNSUCCESSFUL_ERASE):
            print("\n   Erase Status -> Unsuccessfule Erase ")
        elif (BL_Erase_Status[0] == SUCCESSFUL_ERASE):
            print("\n   Erase Status -> Successfule Erase ")
        else:
            print("\n   Erase Status -> Unknown Error")
    else:
        print("Timeout !!, Bootloader is not responding")

def Process_CBL_MEM_WRITE_CMD(Data_Len):
    global Memory_Write_All
    BL_Write_Status = 0
    Serial_Data = Read_Serial_Port(Data_Len)
    BL_Write_Status = bytearray(Serial_Data)
    if(BL_Write_Status[0] == FLASH_PAYLOAD_WRITE_FAILED):
        print("\n   Write Status -> Write Failed or Invalid Address ")
    elif (BL_Write_Status[0] == FLASH_PAYLOAD_WRITE_PASSED):
        print("\n   Write Status -> Write Successfule ")
        Memory_Write_All = Memory_Write_All and FLASH_PAYLOAD_WRITE_PASSED
    else:
        print("Timeout !!, Bootloader is not responding")

def Calculate_CRC32(Buffer, Buffer_Length):
    CRC_Value = 0xFFFFFFFF
    for DataElem in Buffer[0:Buffer_Length]:
        CRC_Value = CRC_Value ^ DataElem
        for DataElemBitLen in range(32):
            if(CRC_Value & 0x80000000):
                CRC_Value = (CRC_Value << 1) ^ 0x04C11DB7
            else:
                CRC_Value = (CRC_Value << 1)
    return CRC_Value
    
def Word_Value_To_Byte_Value(Word_Value, Byte_Index, Byte_Lower_First):
    Byte_Value = (Word_Value >> (8 * (Byte_Index - 1)) & 0x000000FF)
    return Byte_Value

def CalulateBinFileLength():
    BinFileLength = os.path.getsize("Application.bin")
    return BinFileLength

def OpenBinFile():
    global BinFile
    BinFile = open('Application.bin', 'rb')

def Decode_CBL_Command(Command):
    BL_Host_Buffer = []
    BL_Return_Value = 0
    
    ''' Clear the bootloader host buffer '''
    for counter in range(255):
        BL_Host_Buffer.append(0)
    
    if(Command == 1):
        print("Request the bootloader version")
        CBL_GET_VER_CMD_Len = 6
        BL_Host_Buffer[0] = CBL_GET_VER_CMD_Len - 1
        BL_Host_Buffer[1] = CBL_GET_VER_CMD
        CRC32_Value = Calculate_CRC32(BL_Host_Buffer, CBL_GET_VER_CMD_Len - 4)
        CRC32_Value = CRC32_Value & 0xFFFFFFFF
        print("Host CRC = ", hex(CRC32_Value))
        BL_Host_Buffer[2] = Word_Value_To_Byte_Value(CRC32_Value, 1, 1)
        BL_Host_Buffer[3] = Word_Value_To_Byte_Value(CRC32_Value, 2, 1)
        BL_Host_Buffer[4] = Word_Value_To_Byte_Value(CRC32_Value, 3, 1)
        BL_Host_Buffer[5] = Word_Value_To_Byte_Value(CRC32_Value, 4, 1)
        Write_Data_To_Serial_Port(BL_Host_Buffer[0], 1)
        for Data in BL_Host_Buffer[1 : CBL_GET_VER_CMD_Len]:
            Write_Data_To_Serial_Port(Data, CBL_GET_VER_CMD_Len - 1)
        Read_Data_From_Serial_Port(CBL_GET_VER_CMD)
    elif (Command == 2):
        print("Read the commands supported by the bootloader")
        CBL_GET_HELP_CMD_Len = 6
        BL_Host_Buffer[0] = CBL_GET_HELP_CMD_Len - 1
        BL_Host_Buffer[1] = CBL_GET_HELP_CMD
        CRC32_Value = Calculate_CRC32(BL_Host_Buffer, CBL_GET_HELP_CMD_Len - 4)
        CRC32_Value = CRC32_Value & 0xFFFFFFFF
        BL_Host_Buffer[2] = Word_Value_To_Byte_Value(CRC32_Value, 1, 1)
        BL_Host_Buffer[3] = Word_Value_To_Byte_Value(CRC32_Value, 2, 1)
        BL_Host_Buffer[4] = Word_Value_To_Byte_Value(CRC32_Value, 3, 1)
        BL_Host_Buffer[5] = Word_Value_To_Byte_Value(CRC32_Value, 4, 1)
        Write_Data_To_Serial_Port(BL_Host_Buffer[0], 1)
        for Data in BL_Host_Buffer[1 : CBL_GET_HELP_CMD_Len]:
            Write_Data_To_Serial_Port(Data, CBL_GET_HELP_CMD_Len - 1)
        Read_Data_From_Serial_Port(CBL_GET_HELP_CMD)
    elif (Command == 3):
        print("Read the MCU chip identification number")
        CBL_GET_CID_CMD_Len = 6
        BL_Host_Buffer[0] = CBL_GET_CID_CMD_Len - 1
        BL_Host_Buffer[1] = CBL_GET_CID_CMD
        CRC32_Value = Calculate_CRC32(BL_Host_Buffer, CBL_GET_CID_CMD_Len - 4)
        CRC32_Value = CRC32_Value & 0xFFFFFFFF
        BL_Host_Buffer[2] = Word_Value_To_Byte_Value(CRC32_Value, 1, 1)
        BL_Host_Buffer[3] = Word_Value_To_Byte_Value(CRC32_Value, 2, 1)
        BL_Host_Buffer[4] = Word_Value_To_Byte_Value(CRC32_Value, 3, 1)
        BL_Host_Buffer[5] = Word_Value_To_Byte_Value(CRC32_Value, 4, 1)
        Write_Data_To_Serial_Port(BL_Host_Buffer[0], 1)
        for Data in BL_Host_Buffer[1 : CBL_GET_CID_CMD_Len]:
            Write_Data_To_Serial_Port(Data, CBL_GET_CID_CMD_Len - 1)
        Read_Data_From_Serial_Port(CBL_GET_CID_CMD)
    elif (Command == 4):
        print("Read the FLASH Read Protection level")
        CBL_GET_RDP_STATUS_CMD_Len = 6
        BL_Host_Buffer[0] = CBL_GET_RDP_STATUS_CMD_Len - 1
        BL_Host_Buffer[1] = CBL_GET_RDP_STATUS_CMD
        CRC32_Value = Calculate_CRC32(BL_Host_Buffer, CBL_GET_RDP_STATUS_CMD_Len - 4)
        CRC32_Value = CRC32_Value & 0xFFFFFFFF
        BL_Host_Buffer[2] = Word_Value_To_Byte_Value(CRC32_Value, 1, 1)
        BL_Host_Buffer[3] = Word_Value_To_Byte_Value(CRC32_Value, 2, 1)
        BL_Host_Buffer[4] = Word_Value_To_Byte_Value(CRC32_Value, 3, 1)
        BL_Host_Buffer[5] = Word_Value_To_Byte_Value(CRC32_Value, 4, 1)
        Write_Data_To_Serial_Port(BL_Host_Buffer[0], 1)
        for Data in BL_Host_Buffer[1 : CBL_GET_RDP_STATUS_CMD_Len]:
            Write_Data_To_Serial_Port(Data, CBL_GET_RDP_STATUS_CMD_Len - 1)
        Read_Data_From_Serial_Port(CBL_GET_RDP_STATUS_CMD)
    elif (Command == 5):
        print("Jump bootloader to specified address command")
        CBL_GO_TO_ADDR_CMD_Len = 10
        CBL_Jump_Address = input("\n   Please Enter the Address in Hex : ")
        CBL_Jump_Address = int(CBL_Jump_Address, 16)
        BL_Host_Buffer[0] = CBL_GO_TO_ADDR_CMD_Len - 1
        BL_Host_Buffer[1] = CBL_GO_TO_ADDR_CMD
        BL_Host_Buffer[2] = Word_Value_To_Byte_Value(CBL_Jump_Address, 1, 1) 
        BL_Host_Buffer[3] = Word_Value_To_Byte_Value(CBL_Jump_Address, 2, 1) 
        BL_Host_Buffer[4] = Word_Value_To_Byte_Value(CBL_Jump_Address, 3, 1) 
        BL_Host_Buffer[5] = Word_Value_To_Byte_Value(CBL_Jump_Address, 4, 1)
        CRC32_Value = Calculate_CRC32(BL_Host_Buffer, CBL_GO_TO_ADDR_CMD_Len - 4) 
        CRC32_Value = CRC32_Value & 0xFFFFFFFF
        BL_Host_Buffer[6] = Word_Value_To_Byte_Value(CRC32_Value, 1, 1)
        BL_Host_Buffer[7] = Word_Value_To_Byte_Value(CRC32_Value, 2, 1)
        BL_Host_Buffer[8] = Word_Value_To_Byte_Value(CRC32_Value, 3, 1)
        BL_Host_Buffer[9] = Word_Value_To_Byte_Value(CRC32_Value, 4, 1)
        Write_Data_To_Serial_Port(BL_Host_Buffer[0], 1)
        for Data in BL_Host_Buffer[1 : CBL_GO_TO_ADDR_CMD_Len]:
            Write_Data_To_Serial_Port(Data, CBL_GO_TO_ADDR_CMD_Len - 1)
        Read_Data_From_Serial_Port(CBL_GO_TO_ADDR_CMD)
    elif (Command == 6):
        print("Mass erase or sector erase of the user flash command")
        CBL_FLASH_ERASE_CMD_Len = 11
        SectorNumber = 0
        NumberOfSectors = 0
        BL_Host_Buffer[0] = CBL_FLASH_ERASE_CMD_Len - 1
        BL_Host_Buffer[1] = CBL_FLASH_ERASE_CMD
        SectorNumber = input("\n   Please enter start Adress          : ")
        SectorNumber = int(SectorNumber, 16)
        if(SectorNumber != 0xFF):
            NumberOfSectors = int(input("\n   Please enter number of pages to erase (12 Max): "), 16)

        BL_Host_Buffer[2] = Word_Value_To_Byte_Value(SectorNumber, 1, 1)
        BL_Host_Buffer[3] = Word_Value_To_Byte_Value(SectorNumber, 2, 1)
        BL_Host_Buffer[4] = Word_Value_To_Byte_Value(SectorNumber, 3, 1)
        BL_Host_Buffer[5] = Word_Value_To_Byte_Value(SectorNumber, 4, 1)

        BL_Host_Buffer[6] = NumberOfSectors
        CRC32_Value = Calculate_CRC32(BL_Host_Buffer, CBL_FLASH_ERASE_CMD_Len - 4) 
        CRC32_Value = CRC32_Value & 0xFFFFFFFF
        BL_Host_Buffer[7] = Word_Value_To_Byte_Value(CRC32_Value, 1, 1)
        BL_Host_Buffer[8] = Word_Value_To_Byte_Value(CRC32_Value, 2, 1)
        BL_Host_Buffer[9] = Word_Value_To_Byte_Value(CRC32_Value, 3, 1)
        BL_Host_Buffer[10] = Word_Value_To_Byte_Value(CRC32_Value, 4, 1)
        Write_Data_To_Serial_Port(BL_Host_Buffer[0], 1)
        for Data in BL_Host_Buffer[1 : CBL_FLASH_ERASE_CMD_Len]:
            Write_Data_To_Serial_Port(Data, CBL_FLASH_ERASE_CMD_Len - 1)
        Read_Data_From_Serial_Port(CBL_FLASH_ERASE_CMD)
    elif (Command == 7):
        print("Write data into different memories of the MCU command")
        global Memory_Write_Is_Active
        global Memory_Write_All
        File_Total_Len = 0
        BinFileRemainingBytes = 0
        BinFileSentBytes = 0
        BaseMemoryAddress = 0
        BinFileReadLength = 0
        Memory_Write_All = 1
        
        ''' Get the total length of the binary file '''
        File_Total_Len = CalulateBinFileLength()
        print("   Preparing writing a binary file with length (", File_Total_Len, ") Bytes")
        ''' Open the binary file '''
        OpenBinFile()
        ''' Calculate the remaining payload '''
        BinFileRemainingBytes = File_Total_Len - BinFileSentBytes
        ''' Get the start address to write the payload '''
        BaseMemoryAddress = input("\n   Enter the start address : ")
        BaseMemoryAddress = int(BaseMemoryAddress, 16)
        ''' Keep sending the write packet till the last payload byte '''
        while(BinFileRemainingBytes):
            ''' Memory write is active '''
            Memory_Write_Is_Active = 1
            
            ''' Read 128 bytes from the binary file each time '''
            if(BinFileRemainingBytes >= 64):
                BinFileReadLength = 64
            else:
                BinFileReadLength = BinFileRemainingBytes
            
            for BinFileByte in range(BinFileReadLength):
                BinFileByteValue = BinFile.read(1)
                BinFileByteValue = bytearray(BinFileByteValue)
                BL_Host_Buffer[7 + BinFileByte] = int(BinFileByteValue[0])
            
            ''' Update the Host packet with the command code ID '''
            BL_Host_Buffer[1] = CBL_MEM_WRITE_CMD
        
            ''' Update the Host packet with the base address '''
            BL_Host_Buffer[2] = Word_Value_To_Byte_Value(BaseMemoryAddress, 1, 1)
            BL_Host_Buffer[3] = Word_Value_To_Byte_Value(BaseMemoryAddress, 2, 1)
            BL_Host_Buffer[4] = Word_Value_To_Byte_Value(BaseMemoryAddress, 3, 1)
            BL_Host_Buffer[5] = Word_Value_To_Byte_Value(BaseMemoryAddress, 4, 1)
            
            ''' Update the Host packet with the payload length '''
            BL_Host_Buffer[6] = BinFileReadLength
            
            ''' Update the Host packet with the packet length '''
            CBL_MEM_WRITE_CMD_Len = (BinFileReadLength + 11)
            BL_Host_Buffer[0] = CBL_MEM_WRITE_CMD_Len - 1
            
            ''' Update the Host packet with the calculated CRC32 '''
            CRC32_Value = Calculate_CRC32(BL_Host_Buffer, CBL_MEM_WRITE_CMD_Len - 4) 
            CRC32_Value = CRC32_Value & 0xFFFFFFFF
            BL_Host_Buffer[7 + BinFileReadLength] = Word_Value_To_Byte_Value(CRC32_Value, 1, 1)
            BL_Host_Buffer[8 + BinFileReadLength] = Word_Value_To_Byte_Value(CRC32_Value, 2, 1)
            BL_Host_Buffer[9 + BinFileReadLength] = Word_Value_To_Byte_Value(CRC32_Value, 3, 1)
            BL_Host_Buffer[10+ BinFileReadLength] = Word_Value_To_Byte_Value(CRC32_Value, 4, 1)
            
            ''' Calculate the next Base memory address '''
            BaseMemoryAddress = BaseMemoryAddress + BinFileReadLength
            
            ''' Send the packet length to the bootloader '''
            Write_Data_To_Serial_Port(BL_Host_Buffer[0], 1)
            
            ''' Send the complete packet to the bootloader '''
            for Data in BL_Host_Buffer[1 : CBL_MEM_WRITE_CMD_Len]:
                Write_Data_To_Serial_Port(Data, CBL_MEM_WRITE_CMD_Len - 1)
            
            ''' Update the total number of bytes sent to the bootloader '''
            BinFileSentBytes = BinFileSentBytes + BinFileReadLength
            
            ''' Calculate the remaining payload '''
            BinFileRemainingBytes = File_Total_Len - BinFileSentBytes
            print("\n   Bytes sent to the bootloader :{0}".format(BinFileSentBytes))
            
            ''' Read the response from the bootloader '''
            BL_Return_Value = Read_Data_From_Serial_Port(CBL_MEM_WRITE_CMD)
            sleep(0.1)
        ''' Memory write is inactive '''
        Memory_Write_Is_Active = 0
        if(Memory_Write_All == 1):
            print("\n\n Payload Written Successfully")
            
        

SerialPortName = input("Enter the Port Name of your device(Ex: COM3):")
Serial_Port_Configuration(SerialPortName)
        
while True:
    print("\nSTM32F407 Custome BootLoader")
    print("==============================")
    print("Which command you need to send to the bootLoader :");
    print("   CBL_GET_VER_CMD              --> 1")
    print("   CBL_GET_HELP_CMD             --> 2")
    print("   CBL_GET_CID_CMD              --> 3")
    print("   CBL_GET_RDP_STATUS_CMD       --> 4")
    print("   CBL_GO_TO_ADDR_CMD           --> 5")
    print("   CBL_FLASH_ERASE_CMD          --> 6")
    print("   CBL_MEM_WRITE_CMD            --> 7")
    
    CBL_Command = input("\nEnter the command code : ")
    
    if(not CBL_Command.isdigit()):
        print("   Error !!, Please enter a valid command !! \n")
    else:
        Decode_CBL_Command(int(CBL_Command))
    
    input("\nPlease press any key to continue ...")
    Serial_Port_Obj.reset_input_buffer()