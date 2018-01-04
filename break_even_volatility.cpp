#include "break_even_volatility.hpp"

/*******************************************
   *time series constructor & destructor*
********************************************/

time_series::time_series(const std::string& filepath, const std::string& dataname)
:m_filepath(filepath), m_dataname(dataname)
{
    // read data from the csv file (with column 1 as dates and column 2 as data)
    std::ifstream file(m_filepath);
    std::string line;
    std::string cell;
    while(std::getline(file,line)) // do if there is a line to read
    {
        std::stringstream lineStream(line); // convert the line from a string to a stringstream
        
        // date part
        std::getline(lineStream,cell,','); // read the first part (separating the stringstream by ',')
        m_datadate.push_back(c_str_timet(cell)); // store all the dates (in time_t) in the private member m_datadate
        
        // data part
        std::getline(lineStream,cell,','); // read the second part of the stringstream
        m_data.push_back(std::stod(cell)); // store all data (in double) in the private member m_data
    }
}

time_series::~time_series()
{
}


/*******************************************
         *time series methods*
********************************************/

// method to get private data member m_dataname
std::string time_series::get_dataname() const
{
    return m_dataname;
}

// methods to get the whole series of data
std::vector<double> time_series::get_data() const
{
    return m_data;
}

std::vector<time_t> time_series::get_date() const
{
    return m_datadate;
}

// method to determine the starting and ending positions of the target data series
std::vector<ptrdiff_t> time_series::get_datapos(const std::string& maturity, const size_t& term_day) const
{
    std::vector<ptrdiff_t> result(2);
    time_t maturityt = c_str_timet(maturity); // convert the maturity time string to time_t object
    ptrdiff_t endpos = std::find(m_datadate.begin(), m_datadate.end(), maturityt) - m_datadate.begin(); // find the position of maturity date
    if (endpos >= m_datadate.size()) // check if the end date is in the data series
    {
        std::cout << "Target date is not found in the data!!" << std::endl;
        return result; // !!TO DO: Replace by error!
    }
    else
    {
        ptrdiff_t startpos;
        int daycount = 0;
        bool startfound = false;
        while(daycount <= 7)
        {
            time_t startt = maturityt - (term_day + daycount)*24*60*60; // define the starting date by end date - number of calendar days (- daycount if previous date doesn't exist in the data series)
            startpos = std::find(m_datadate.begin(), m_datadate.end(), startt) - m_datadate.begin();
            if (startpos < m_datadate.size())
            {
                startfound = true;
                break;
            }
            daycount += 1;
        }
        
        if(startfound == false)
        {
            std::cout << "Not enough historical data!!" << std::endl;
            return result; // !!TO DO: Replace by error!
        }
        else
        {
            result = {startpos, endpos};
            return result;
        }
    }
}

// method to get a part of the series (data) with starting and ending positions
std::vector<double> time_series::get_data(const ptrdiff_t& startpos, const ptrdiff_t& endpos) const
{
    std::vector<double> result(m_data.begin() + startpos, m_data.begin() + endpos + 1);
    return result;
}

// method to get a part of the series (date) with starting and ending positions
std::vector<time_t> time_series::get_date(const ptrdiff_t& startpos, const ptrdiff_t& endpos) const
{
    std::vector<time_t> result(m_datadate.begin() + startpos, m_datadate.begin() + endpos + 1);
    return result;
}

/*
// method to create a vector containing daily days to maturity
std::vector<size_t> day_to_maturity(const size_t& day_number) const
{
    std::vector<size_t> result(day_number + 1); // create a vector with size day_number+1
    std::iota(result.begin(), result.end(), 0); // fill the vector with increasing number from 0,1,2,3,... to day_number
    std::for_each(result.begin(), result.end(), [](size_t& x) {x = day_number - x;}); // revert the vector from ascending order to descending order
    return result;
}
*/

/*******************************************
     *pricer constructor & destructor*
********************************************/

pricer::pricer(const time_series& underlying, const double& strike, const double& vol, const time_series& rate, const std::string& maturity, const size_t& term_day)
: m_underlying(underlying), m_strike(strike), m_vol(vol), m_rate(rate), m_maturity(maturity), m_term_day(term_day)
{
    // create a vector of interest rate data with dates match with those of the underlying data
    m_datapos = m_underlying.get_datapos(m_maturity, m_term_day);
    std::vector<ptrdiff_t> ratedatapos = m_rate.get_datapos(m_maturity, m_term_day);
    
    std::vector<time_t> datadate = m_underlying.get_date(m_datapos[0], m_datapos[1]);
    std::vector<time_t> ratedate = m_rate.get_date(ratedatapos[0], ratedatapos[1]);
    
    std::vector<double> ratedata = m_rate.get_data(ratedatapos[0], ratedatapos[1]);
    
    ptrdiff_t targetpos;
    time_t targetdate;
    
    for(int i = 0; i < m_underlying.size(); i++)
    {
        int daycount = 0;
        bool targetfound = false;
        while(daycount <= 7)
        {
            targetdate = datadate[i] - daycount*24*60*60;
            targetpos = std::find(ratedate.begin(), ratedate.end(), targetdate) - ratedate.begin();
            if (targetpos < m_datadate.size())
            {
                startfound = true;
                break;
            }
            daycount += 1;
        }
        if(targetfound == false)
        {
            std::cout << "Missing interest rate data!" << std::endl;
            m_fixedrate.push_back(0); // !!TO DO: Replace by error!
        }
        else
        {
            m_fixedrate.push_back(ratedata[targetpos]);
        }
    }
}

pricer::~pricer()
{
}

std::vector<double> pricer::BS_price() const
{
    /*std::vector<size_t> time_to_maturity = m_underlying.day_to_maturity(m_maturity)
    
    std::vector<double> timeToMaturity = 
    double d1 = 1/(m_vol)*/
}


// function converting date string to time_t object
time_t c_str_timet(const std::string& targetdate)
{
    struct tm temp = {0};
    
    std::istringstream tt(targetdate); // convert the date string to stringstream
    tt >> std::get_time(&temp, "%d/%m/%Y"); // convert the date stringstream to struct tm
    
    time_t result = std::mktime(&temp);
    return result; // convert struct tm to time_t
}

