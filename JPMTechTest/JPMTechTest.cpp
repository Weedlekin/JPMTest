/*
  This code uses Boost 1.63. It will not run without it.
  Note also that a C++ 11 compiler is required. This example
  will not compile on earlier versions.
*/

#if defined _MSC_VER
#include "stdafx.h"
#endif

#include <string>
#include <iostream>
#include <ctime>
#include <exception>
#include <vector>
#include <algorithm>
// Boost date handling routines
#include <boost/date_time/gregorian/gregorian.hpp>

// Class to handle settlement dates, adjusting for weekends that are
// correct for te currency
class SettlementDateClass
{
private:
    std::string stringDate;
    std::string currency;

    // Many useful mechanisms for manipulating dates. 
    boost::gregorian::date gregorianDate;

    // Return an integer between 1 and 12 that corresponds to the month number
    // for a month string in a Transaction
    int MonthToInt(std::string & monthStr)
    {
        if (monthStr == "Jan")
        {
            return 1;
        }
        else if (monthStr == "Feb")
        {
            return 2;
        }
        else if (monthStr == "Mar")
        {
            return 3;
        }
        else if (monthStr == "Apr")
        {
            return 4;
        }
        else if (monthStr == "May")
        {
            return 5;
        }
        else if (monthStr == "Jun")
        {
            return 6;
        }
        else if (monthStr == "Jul")
        {
            return 7;
        }
        else if (monthStr == "Aug")
        {
            return 8;
        }
        else if (monthStr == "Sep")
        {
            return 9;
        }
        else if (monthStr == "Oct")
        {
            return 10;
        }
        else if (monthStr == "Nov")
        {
            return 11;
        }
        else
        {
            return 12;
        }
    }

    // Calculates the next working day. Will be today unless it's a weekend,
    // in which case it will work out the next working day for a currency, and set
    // yearMonthDay accordingly
    void CalcWorkingDay()
    {
        auto dayOfWeek = gregorianDate.day_of_week();
        auto dayNum = dayOfWeek.as_number();

        int amountToAdd = 0;

        if ((currency == "AED") || (currency == "SAR"))
        {
            // Friday
            if (dayNum == 5)
            {
                amountToAdd += 2;
            }
            // Saturday
             else if (dayNum == 6)
            {
                amountToAdd++;
            }

            // Add the weekend offset to the date
            boost::gregorian::date_duration duration{ amountToAdd };
            gregorianDate += duration;
        }
        else // Other currencies
        {
            // Saturday
            if (dayNum == 6)
            {
                amountToAdd+=2;
            }
            // Sunday
            else if (dayNum == 0)
            {
                amountToAdd++;
            }

            // Add the weekend offset to the date
            boost::gregorian::date_duration duration{ amountToAdd };
            gregorianDate += duration;
        }
    }

    // Simplistic parser which assumes that dates are in the format:
    // 01 Jan 1999. Production code would obviously handle a lot
    // more date formats, and throw exceptions for invalid ones.
    void ParseDate()
    {
        // Day, month, year. Parsed values will be put into these variables.
        unsigned short day, month, year;
        std::string substr;

        // Look for separator (i.e. a space)
        size_t spacePos = stringDate.find(" ");
        // Copy the substring before the separator, and then convert it to
        // an integer. This is the day portion of the date.
        substr = stringDate.substr(0, spacePos);
        day = std::atoi(substr.c_str());

        // Month string
        size_t nextSpace = stringDate.find(" ", spacePos + 1);
        substr = stringDate.substr(spacePos + 1, nextSpace - spacePos - 1);
        month = MonthToInt(substr);
        spacePos = nextSpace;

        // Year
        substr = stringDate.substr(spacePos);
        year = std::atoi(substr.c_str());

        // Construct a Gregoran date object from the parsed values.
        gregorianDate = boost::gregorian::date{ year, month, day};

        CalcWorkingDay();
    }

public:
    SettlementDateClass() = delete;
    SettlementDateClass(std::string & sDate, std::string & sCurrency):
        stringDate(sDate),
        currency(sCurrency)
    {
        ParseDate();
    }

    boost::gregorian::date GetDate()
    {
        return gregorianDate;
    }

    // Formats date as dd <long month> yyyy, e.g. 01 Jun 2020
    std::string FormatDate()
    {
        char *monthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
            "July", "Aug", "Sept", "Oct", "Nov", "Dec"};

        unsigned short day = gregorianDate.day();
        unsigned short month = gregorianDate.month();
        unsigned short year = gregorianDate.year();

        return std::to_string(day) + " " + monthNames[month - 1] + " " + std::to_string(year);
    }
};

// Represents a single  buy or sell order
class Transaction
{
private:
    std::string entity;
    char buySell;
    double agreedFx; // Should really be a Boost decimal because doubles are
                     // prone to binary rounding errors.
    std::string currency;
    std::string instructionDateString;
    std::string settlementDateString;
    int units;
    double pricePerUnit; // same as agreedFx
 
    // Check to see whether the values passed to the ctor are valid.
    // Note that these checks are currently crude; production code
    // would of course be significantly more thorough.
    void Validate()
    {
        if (entity.length() == 0)
        {
            throw std::exception("Entity cannot be empty");
        }

        if ((buySell != 'B') && (buySell != 'S'))
        {
            throw std::exception("Buy / Sell must be 'B' or 'S'");
        }

        if (agreedFx <= 0.0)
        {
            throw std::exception("AgreedFx may not be zero or nrgative");
        }

        if (currency.length() != 3)
        {
            throw std::exception("Currency must have 3 characters");
        }

        if (instructionDateString.length() == 0)
        {
            throw std::exception("InstructionDate cannot be empty");
        }

        if (settlementDateString.length() == 0)
        {
            throw std::exception("SettlementDate cannot be empty");
        }

        if (units <= 0)
        {
            throw std::exception("Cannot be 0 or negative");
        }

        if (pricePerUnit <= 0.0)
        {
            throw std::exception("AgreedFx may not be zero or nrgative");
        }
    }

public:
    Transaction() = delete;

    // Construct a Transaction, passing the relevant parameters
    Transaction(char * sEntity, char cBuySell, double dAgreedFx, char * sCurrency,
        char * sInstructionDate, char * sSettlementDate, int iUnits,
        double dPricePerUnit) :
        entity(sEntity),
        buySell(cBuySell),
        agreedFx(dAgreedFx),
        currency(sCurrency),
        instructionDateString(sInstructionDate),
        settlementDateString(sSettlementDate),
        units(iUnits),
        pricePerUnit(dPricePerUnit)
    {
        // Validate fields
        Validate();
    }

    // Tests to see whether an object is equivalent 
    bool operator == (const Transaction & trans)
    {
        return ((entity == trans.entity) && (buySell == trans.buySell) && (agreedFx == trans.agreedFx)
            && (currency == trans.currency) && (instructionDateString == trans.instructionDateString)
            && (settlementDateString == trans.settlementDateString) && (units == trans.units)
            && (pricePerUnit == trans.pricePerUnit));
    }

    // Accessor methods. These are read-only.
    std::string Entity() {return entity; }
    char BuySell() { return buySell; }
    double AgreedFx() { return agreedFx; }
    std::string Currency() {return currency; }
    std::string InstructionDate() { return instructionDateString; }
    
    // This method returns a settlement date adjusted for weekends.
    std::string SettlementDate()
    {
        SettlementDateClass sdc(settlementDateString, currency);
        return sdc.FormatDate();
    }

    int Units() {return units; }
    double PricePerUnit() {return pricePerUnit; }
  
    double DollarPrice()
    {
        return agreedFx * pricePerUnit * units;
    }
};

class TransactionReport
{
private:
    std::vector<Transaction> outgoing;
    std::vector<Transaction> incoming;
    void AddOutgoing(Transaction &trans)
    {
        if ((outgoing.size() == 0) || (outgoing.back().SettlementDate() <= trans.SettlementDate()))
        {
            outgoing.push_back(trans);
        }
        else
        {
            outgoing.insert(outgoing.begin(), trans);
        }
    }

    void AddIncoming(Transaction & trans)
    {
        if ((incoming.size() == 0) || (incoming.back().SettlementDate() <= trans.SettlementDate()))
        {
            incoming.push_back(trans);
        }
        else
        {
            incoming.insert(incoming.begin(), trans);
        }
    }

public:
    void AddTransaction(Transaction & trans)
    {
        if (trans.BuySell() == 'B')
        {
            AddOutgoing(trans);
        }
        else
        {
            AddIncoming(trans);
        }
          
    }

    void GenReport()
    {
        GenOutgoingReport();
        GenIncomingReport();
    }

    // Report for Buy orders
    void GenOutgoingReport()
    {
        std::string currDate = "";
        double total = 0;

        for (auto &trans : outgoing)
        {
            if (trans.SettlementDate() == currDate)
            {
                total += trans.DollarPrice();
            }
            else
            {
                if (total != 0)
                {
                    std::cout << "Outgoing total for " << currDate << " = " << total << std::endl;
                }

                currDate = trans.SettlementDate();
                total = trans.DollarPrice();
            }
        }

        std::cout << "Outgoing total for " << currDate << " = " << total << std::endl << std::endl;
    }

    // Report for Sell orders
    void GenIncomingReport()
    {
        std::string currDate = "";
        double total = 0;

        for (auto &trans : incoming)
        {
            if (trans.SettlementDate() == currDate)
            {
                total += trans.DollarPrice();
            }
            else
            {
                if (total != 0)
                {
                    std::cout << "Incoming total for " << currDate << " = " << total << std::endl;
                }

                currDate = trans.SettlementDate();
                total = trans.DollarPrice();
            }
        }

        std::cout << "Incoming total for " << currDate << " = " << total << std::endl;
    }
};

int main()
{
    char buf[20];
    TransactionReport rep;
    rep.AddTransaction(Transaction("XYZ", 'B', 1.2, "GBP", "1 Mar 2017", "3 Mar 2017", 200, 18.1));
    rep.AddTransaction(Transaction("DEF", 'B', 0.5, "SGP", "27 Feb 2017", "1 Mar 2017", 120, 4.5));
    rep.AddTransaction(Transaction("GHI", 'B', 0.81, "EUR", "27 Feb 2017", "1 Mar 2017", 220, 7));

    rep.AddTransaction(Transaction("ABC", 'S', 0.81, "EUR", "28 Feb 2017", "1 Mar 2017", 80, 11.25));
    rep.AddTransaction(Transaction("FFO", 'S', 0.2, "AED", "2 Mar 2017", "3 Mar 2017", 225, 19));

    rep.GenReport();

    std::cout << std::endl << "Press an alphanumeric key followed by enter..." << std::endl;
    std::cin >> buf;

    return 0;
}

