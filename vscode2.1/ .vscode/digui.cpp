
#include "square.h"
void fill(struct _ngg * pngg,int pos);
int NN, N;

int main(int argc, char *argv[])
{
    int groupSize, totals, i;
    SQUARE s[MAXNN];
    std::thread t[MAXNN];
    int sumtotalCount, sumpartResultCount, sumresultCount;

    printf("input a number(3-5) and group size: ");
    scanf("%d,%d", &N, &groupSize);

    NN = N * N;
    fill(&squarelist,0);
    if (N <= 0 || N > 9 || groupSize <= 0 || groupSize > NN)
    {
        printf("N: 3~9;  group size: 1~N*N");
        return -1;
    }

    totals = NN / groupSize;
    if (NN % groupSize)
        totals++;

    for (i = 0; i < totals; i++)
    {
        initSquare(&(s[i]), i * groupSize + 1, std::min(i * groupSize + groupSize, NN));
    }

    for (i = 0; i < totals; i++)
    {
        t[i] = std::thread(beginWorker, &(s[i]));
    }

    printf("all         finish  find time\r\n");

    int finished = false;
    while (!finished)
    {
        sumresultCount = 0;
        sumpartResultCount = 0;
        sumtotalCount = 0;

        for (i = 0; i < totals; i++)
        {
            finished |= s[i].working;
            sumresultCount += s[i].resultCount;
            sumpartResultCount += s[i].partResultCount;
            sumtotalCount += s[i].totalCount;
        }
        finished = !finished;

        printf("%-16d %-12d  %-8d %.1f\r", sumtotalCount, sumpartResultCount, sumresultCount, (clock() - s[0].t_start) / 1000.0);

        _sleep(1000);

        if (_kbhit())
        {
            break;
        }
    }
    printf("\r\n");

    char filename[256];
    sprintf(filename, "%dx%d result.txt", N, N);
    fileWrite(filename, s, totals );

    for(i=0;i<totals;i++)
        sl_free(&(s[i].head));

    getchar();
}

void beginWorker(SQUARE *ps)
{
    ps->working = true;
    ps->t_start = clock();

    int i;

    for (i = ps->scope_start; i <= ps->scope_end; i++)
    {
        ps->i[0] = i;
        ps->remain[i] = 0;
        ps->rsum[0] = i;
        ps->csum[0] = i;
        msquare(ps, 1);
        ps->remain[i] = 1;
    }

    ps->working = false;
}

void msquare(SQUARE *ps, int no)
{
    int i;
    int r, c, rsum, csum, tempcsum, temprsum;

    r = no / N;
    c = no % N;

    rsum = ps->rsum[r];
    csum = ps->csum[c];

    

    for (i = 1; i <= NN; i++)
    {
        if (ps->remain[i] == 0)
            continue;

        tempcsum = csum + i;
        temprsum = rsum + i;

        if (temprsum > ps->stdsum || tempcsum > ps->stdsum)
            break;

        if (c == N - 1 && temprsum != ps->stdsum)
            continue;
        if (r == N - 1 && tempcsum != ps->stdsum)
            continue;

        if (c > 1)
        {
            if ((temprsum + ps->threshold_max[c] < ps->stdsum) || temprsum + ps->threshold_min[c] > ps->stdsum)
                continue;
        }
        if (r > 1)
        {
            if ((tempcsum + ps->threshold_max[r] < ps->stdsum) || tempcsum + ps->threshold_min[r] > ps->stdsum)
                continue;
        }
        ps->totalCount++;

        ps->i[no] = i;
        ps->remain[i] = 0;
        ps->rsum[r] = temprsum;
        ps->csum[c] = tempcsum;

        if (no == NN - 1)
        {
            ps->partResultCount++;
            if (exam(ps))
            {
                ps->resultCount++;
                ps->time = clock() - ps->t_start;
                sl_append(ps);
                
        }
        else
        {
            msquare(ps, no + 1);
        }
        ps->remain[i] = 1;
        ps->i[no] = 0;
       
    }
    ps->rsum[r] = rsum;
    ps->csum[c] = csum;
}

void initSquare(SQUARE *ps, int start, int end)
{
    int i, temp;

    memset(ps->i, 0, sizeof(ps->i));
    ps->head.next = NULL;
    ps->tail = &(ps->head);

    ps->t_start = clock();
    ps->resultCount = ps->totalCount = ps->partResultCount = 0;
    ps->scope_start = start;
    ps->scope_end = end;

    temp = 0;
    
{
    ps->stdsum = temp / N;
    ps->time = 0;

    for (i = 0; i < N; i++)
    {
        ps->csum[i] = 0;
        ps->rsum[i] = 0;
    }

    calcThreshold(ps);
}

SQUARENODE *sl_append(SQUARE *ps)
{
    SQUARENODE *newnode;

    newnode = new SQUARENODE;

    memcpy(&(newnode->i), ps->i, sizeof(int) * NN + 1);
    newnode->partResultCount = ps->partResultCount;
    newnode->time = ps->time;
    newnode->resultCount = ps->resultCount;
    newnode->totalCount = ps->totalCount;

    newnode->next = ps->tail->next;
    ps->tail->next = newnode;
    ps->tail = newnode;

    return newnode;
}

void sl_free(SQUARENODE *head)
{
    SQUARENODE *node;

    while (head->next != NULL)
    {
        node = head->next;
        head->next = node->next;
        delete node;
    }
}

void fileWrite(char *filename, SQUARE *ps, int count )
{
    FILE *fp;
    SQUARENODE *n;
    char line[4096], temp[1024];
    int i, l, serialnum = 1;

    fopen_s(&fp, filename, "w");

    for (l = 0; l < count; l++)
    {
        n = ps[l].head.next;
        sprintf(line, "\r\nthread %d, scope:%d~%d, ms used:%.3f\r\n", l + 1,
                ps[l].scope_start, ps[l].scope_end, ps[l].time / 1000.0);
        fwrite(line, strlen(line), 1, fp);

        while (n)
        {
            sprintf(line, "%6d [ ", serialnum++);
            for (i = 0; i < NN; i++)
            {
                sprintf(temp, "%2d ", n->i[i]);
                strcat_s(line, sizeof(line), temp);
            }
            strcat_s(line, sizeof(line), "] ");
            sprintf(temp, "%.3f  %ld  %ld\n", n->time / 1000.0, n->totalCount, n->partResultCount);
            strcat_s(line, sizeof(line), temp);

            fwrite(line, strlen(line), 1, fp);

            n = n->next;
        }
    }
    fclose(fp);
}

void calcThreshold(SQUARE *ps)
{
    int temp, i, count;

    
    count = N - 1;
    ps->threshold_max[count] = 0;
    for (i = 1; i < NN; i++)
    {
        temp = NN - i + 1;
        if (ps->remain[temp]==0)
            continue;
        ps->threshold_max[count - 1] = temp + ps->threshold_max[count];
        if (--count == 0)
            break;
    }

    
    count = N - 1;
    ps->threshold_min[count] = 0;
    for (i = 1; i < NN; i++)
    {
        temp = i;
        if (ps->remain[temp]==0)
            continue;
        ps->threshold_min[count - 1] = temp + ps->threshold_min[count];
        if ( --count == 0 )
            break;
    }
}


int exam(SQUARE *ps)
{
    int j;
    int sum1, sum2;

    
    sum1 = 0;
    sum2 = 0;

    
    for (j = 0; j < N; j++)
    {
        sum1 += ps->i[j * N + j];         
        sum2 += ps->i[j * N + N - 1 - j]; 
    }

    
    if (sum1 != ps->stdsum || ps->stdsum != sum2)
        return 0;
    else
        return 1;
}
void fill(struct _squarelist * pngg,int pos)
{

 int flag = getFlag(pos);  
 int i;
 
 for(i = 1; i <=NN; i++)
 {
  
  if(isFilled(pngg,pos,i))
  {
   continue;
  }
 
  pngg->num[pos] = i;
 
  switch(flag)
  {
  
  case ROW_END:
  
   if(pngg->val == sumRow(pngg,pos))
   {
    fill(pngg,pos+1);
   }
   break;
  
  case COL_END:
  
   if(pngg->val == sumCol(pngg,pos))
   {
    fill(pngg,pos+1);
   }
   break;
  
  case LEFT_DOWN:
  
   if(pngg->val == sumCol(pngg,pos) && 
    pngg->val == sumRU_LD(pngg))
   {
    fill(pngg,pos+1);
   }
   break;
  
  case RIGHT_DOWN:
  
   if(pngg->val == sumRow(pngg,pos) &&
    pngg->val == sumCol(pngg,pos) &&
    pngg->val == sumLU_RD(pngg) )
   {
    prt(pngg);
   }
   break;
  
  default:
   
   fill(pngg,pos+1);
  }
 }
}
