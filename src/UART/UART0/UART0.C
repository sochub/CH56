/********************************** (C) COPYRIGHT *******************************
* File Name          : UART0.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/3/8
* Description        : CH561 UART0 DEMO
*           
*                      (1)������0���ͺͽ������ݣ�ʵ��ͨ��FIFO���ͺͽ�������.
*******************************************************************************/



/******************************************************************************/
/* ͷ�ļ����� */
#include <stdio.h>
#include <string.h>
#include "CH561SFR.H"
#include "SYSFREQ.H"

/******************************************************************************/
/* �������� */
UINT8 SEND_STRING[ ] = { "l am uart0!\n" };
UINT8 SEND_STRING1[ ] = { "IRQ sucess!\n" };
UINT8 buf[ 50 ]; 
UINT8 rcvbuf[ 50 ];

/* ����һ��LED���ڼ����ʾ����Ľ���,�͵�ƽLED�� */
#define LED                     1<<3

#define LED_OUT_INIT(  )     { R32_PB_OUT |= LED; R32_PB_DIR |= LED; }         /* LED �ߵ�ƽΪ������� */
#define LED_OUT_ACT(  )      { R32_PB_CLR |= LED; }                            /* LED �͵�ƽ����LED��ʾ */
#define LED_OUT_INACT(  )    { R32_PB_OUT |= LED; }                            /* LED �ߵ�ƽ�ر�LED��ʾ */

/*******************************************************************************
* Function Name  : IRQ_Handler
* Description    : IRQ�жϺ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

 __irq void IRQ_Handler( void )
{
    while(1);
}
  
/*******************************************************************************
* Function Name  : FIQ_Handler
* Description    : FIQ�жϺ���
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

__irq void FIQ_Handler( void )
{
    while(1);
}

/*******************************************************************************
* Function Name  : Uart0_Init
* Description    : ����0��ʼ��
* Input          : baud-���ڲ����ʣ����Ϊ��Ƶ1/8
* Output         : None
* Return         : None
*******************************************************************************/

void Uart0_Init( UINT32  baud )
{
    UINT32 x;

    x = 10 * FREQ_SYS/ 8 / baud;                                                /* 115200bps */
    x += 5;                                                                     /* �������� */
    x /= 10;
    R8_UART0_LCR = RB_LCR_DLAB;                                                 /* DLABλ��1 */
    R8_UART0_DIV = 1;                                                           /* Ԥ��Ƶ */
    R8_UART0_DLM = x>>8;
    R8_UART0_DLL = x&0xff;              
    R8_UART0_LCR = RB_LCR_WORD_SZ ;                                             /* �����ֽڳ���Ϊ8 */
    R8_UART0_FCR = RB_FCR_FIFO_TRIG|RB_FCR_TX_FIFO_CLR|RB_FCR_RX_FIFO_CLR |    
                   RB_FCR_FIFO_EN ;                                             /* ����FIFO������Ϊ14���巢�ͺͽ���FIFO��FIFOʹ�� */
    R8_UART0_IER = RB_IER_TXD_EN;                                               /* TXD enable */
    R8_UART0_MCR = RB_MCR_OUT1;
    R32_PB_SMT |= RXD0|TXD0;                                                    /* RXD0 schmitt input, TXD0 slow rate */
    R32_PB_PU  |= RXD0;                                                         /* disable pulldown for RXD0, keep pullup */
    R32_PB_DIR |= TXD0;                                                         /* TXD0 ������Ϊ��� */
}

/*******************************************************************************
* Function Name  : fputc
* Description    : ͨ��������������Ϣ
* Input          : c  -writes the character specified by c 
*                  *f -the output stream pointed to by *f
* Output         : None
* Return         : None
*******************************************************************************/

int    fputc( int c, FILE *f )
{
    R8_UART0_THR = c;                                                           /* �������� */
    while( ( R8_UART0_LSR & RB_LSR_TX_FIFO_EMP ) == 0 );                        /* �ȴ����ݷ��� */
    return( c );
}

/*******************************************************************************
* Function Name  : UART0_SendByte
* Description    : ����0�����ֽ��ӳ���
* Input          : dat -Ҫ���͵�����
* Output         : None
* Return         : None
*******************************************************************************/

void UART0_SendByte( UINT8 dat )   
{        
    R8_UART0_THR  = dat;
    while( ( R8_UART0_LSR & RB_LSR_TX_ALL_EMP ) == 0 );                         /* �ȴ����ݷ��� */       
}

/*******************************************************************************
* Function Name  : UART0_SendStr
* Description    : ����0�����ַ����ӳ��� 
* Input          : *str -���͵��ַ�������
* Output         : None
* Return         : None
*******************************************************************************/

void UART0_SendStr( UINT8 *str )   
{
    while( 1 ){
        if( *str == '\0' ) break;
        UART0_SendByte( *str++ );
    }
}

/*******************************************************************************
* Function Name  : UART0_RcvByte
* Description    : ����0����һ�ֽ��ӳ���  
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

UINT8 UART0_RcvByte( void )    
{
    UINT8 Rcvdat = 0;

    if( !( ( R8_UART0_LSR  ) & ( RB_LSR_OVER_ERR |RB_LSR_PAR_ERR  | RB_LSR_FRAME_ERR |  RB_LSR_BREAK_ERR  ) ) ) {
        while( ( R8_UART0_LSR & RB_LSR_DATA_RDY  ) == 0 );                      /* �ȴ�����׼���� */ 
        Rcvdat = R8_UART0_RBR;                                                  /* �ӽ��ջ���Ĵ����������� */ 
    }
    else{
        R8_UART0_RBR;                                                           /* �д������ */
    }
    return( Rcvdat );
}

/*******************************************************************************
* Function Name  : Seril0Send
* Description    : ����FIFO������0���Ͷ��ֽڳ���
* Input          : Data -Ҫ���͵�����
*                  Num  -�������ݵĳ���
* Output         : None
* Return         : None
*******************************************************************************/

void  Seril0Send( UINT8 *Data, UINT8 Num )                        
{
    do{
        while( ( R8_UART0_LSR & RB_LSR_TX_FIFO_EMP ) == 0 );                    /* �ȴ����ݷ������ */ 
        R8_UART0_THR  =*Data++;  
    }while( --Num );
}

/*******************************************************************************
* Function Name  : Seril0Rcv
* Description    : ����FIFO,����0���ն��ֽ��ӳ���
* Input          : buf -�������ݻ�����
* Output         : None
* Return         : RcvNum -�������ݳ���
*******************************************************************************/

UINT8  Seril0Rcv( UINT8 *buf )    
{
    UINT8 RcvNum = 0;

    if( !( ( R8_UART0_LSR  ) & ( RB_LSR_OVER_ERR |RB_LSR_PAR_ERR | RB_LSR_FRAME_ERR | RB_LSR_BREAK_ERR ))){
        while( ( R8_UART0_LSR & RB_LSR_DATA_RDY  ) == 0 );                      /* �ȴ�����׼���� */  
        do{
            *buf++ = R8_UART0_RBR;                                              /* �ӽ��ջ���Ĵ����������� */ 
            RcvNum++;
        }while( (R8_UART0_LSR & RB_LSR_DATA_RDY) == 0x01 );
    }
    else{
        R8_UART0_RBR;
    }
    return( RcvNum );
}

/*******************************************************************************
* Function Name  : UART0Send_FIFO
* Description    : ����FIFO,һ�����16�ֽڣ�����0���Ͷ��ֽ��ӳ���
* Input          : *Data -Ҫ���͵�����ָ��
*                  Num   -����
* Output         : None
* Return         : None
*******************************************************************************/

void UART0Send_FIFO( UINT8 *Data, UINT8 Num ) 
{
    int i;

    while( 1 ){
        while( ( R8_UART0_LSR & RB_LSR_TX_ALL_EMP ) == 0 );                     /* �ȴ����ݷ�����ϣ�THR,TSRȫ�� */ 
        if( Num <= UART0_FIFO_SIZE){
            do{
                R8_UART0_THR = *Data++;
            }while(--Num) ;
            break;
        }
        else{
            for(i=0;i<UART0_FIFO_SIZE;i++){
                R8_UART0_THR = *Data++;
            }
            Num -= UART0_FIFO_SIZE;
        }
    }
}

/*******************************************************************************
* Function Name  : main
* Description    : ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

int main( void ) 
{
    UINT8 i,RcvNum;

    LED_OUT_INIT( );
    LED_OUT_ACT( );                                                             /* ������LED��һ����ʾ���� */
    Delay_ms( 100 );
    LED_OUT_INACT( );
    Uart0_Init( 115200 );                                                       /*  ����0��ʼ�� */ 
    for( i = 0; i < 50; i++ )    buf[ i ] = i;
    UART0_SendStr(SEND_STRING );                                                /* ����0�����ַ��� */
    UART0_SendByte(0xAA);                                                       /* ����0����1�ֽ� */
    UART0Send_FIFO( buf, 50 );                                                  /* ����FIFO������50�ֽ����� */
    while(1){
        RcvNum =Seril0Rcv( rcvbuf );                                            /* �ȴ��������ݣ���ͨ������0���ͳ�ȥ */ 
        Seril0Send( rcvbuf, RcvNum );
    }
}

/*********************************** endfile **********************************/