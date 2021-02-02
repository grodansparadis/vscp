// FILE: canal_config.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/*!
    This is a helper class that can be used for CANAL wizard configuration. The
    mechanism is described here 

    https://grodansparadis.github.io/vscp-doc-canal/#/canalgetdriverinfo

*/

#if !defined(CANALCONFIG_H__INCLUDED_)
#define CANALCONFIG_H__INCLUDED_

#include <string>
#include <list>
#include <vector>


///////////////////////////////////////////////////////////////////////////////
//                                Items
///////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------

class wizardStepBase
{

public:    
    enum class wizardType {NONE,STRING,BOOL,INT32,UINT32,INT64,UINT64,FLOAT,CHOICE};

    wizardType m_type = wizardType::NONE;

 public:
    wizardStepBase();
    ~wizardStepBase();

    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    virtual void setValue(const std::string& strValue) = 0;

    /*!
        Get set value on string form
        @return Value on string form
    */
    virtual std::string getValue(void) = 0;

    /*!
        Get the real value from dialog value (can be index into list etc)
        @param input String that should be handled
        @return Real value on standard string form
    */
    virtual std::string getRealValue(const std::string& input) = 0;

    /*!
        Set description
        @param description Description of this choice
    */
    void setDescription(const std::string description) { m_description = description; };

    /*!
        Get description
        @return Description as standard string
    */
    std::string getDescription(void) { return m_description; };

    /*!
        Set info URL
        @param url Pointer to info url
    */
    void setInfoUrl(const std::string url) { m_infourl = url; };

    /*!
        Get info URL
        @return Pointer to info url as standard string
    */
    std::string getInfoUrl(void) { return m_infourl; };

    /*!
        Set optional
        @param bOptional Set optional flag
    */
    void setOptional(bool bOptional) { m_bOptional = bOptional; };

    /*!
        Return optional flag
        @return Boolean for optional flag
    */
    bool isOptional(void) { return m_bOptional; };
    

 private:   

    /// True if this is an optional item
    bool m_bOptional;

    /// Description of item
    std::string m_description;

    /// Pointer to URL describing the item
    std::string m_infourl;
};

// ----------------------------------------------------------------------------

/*!
    Class that holds a configuration string

    -----------------------------------
    |  label: _________________________
    -----------------------------------
              OK    CANCEL
*/
class wizardStepString : public wizardStepBase
{

 public:
    wizardStepString();
    ~wizardStepString();

    
    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    void setValue(const std::string& strValue) { m_value = strValue; };

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void) { return m_value;}; 

    /*!
        Get the real value from dialog value (can be index into list etc)
        @param input String that should be handled
        @return Real value on standard string form
    */
    virtual std::string getRealValue(const std::string& input) { return getValue(); };

    /*!
        Check if the value is valid
        @return True if value is valid.
    */
    bool isValueValid(const std::string& strValue) { return true; };

 private:

    /// Item value
    std::string m_value;

    /// True if this is an optional item
    bool m_bOptional;

    /// Description of item
    std::string m_description;

    /// Pointer to URL describing the item
    std::string m_infourl;
};

// ----------------------------------------------------------------------------

/*!
    Class that holds a configuration boolean

    -----------------------------------
    |          x checkbox
    -----------------------------------
              OK    CANCEL
*/

class wizardStepBool : public wizardStepBase
{

 public:
    wizardStepBool();
    ~wizardStepBool();

    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    void setValue(const std::string& strValue);

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void); 

    /*!
        Get the real value from dialog value (can be index into list etc)
        @param input String that should be handled
        @return Real value on standard string form
    */
    virtual std::string getRealValue(const std::string& input) { return getValue(); };

    /*!
        Check if the value is valid (true/false/1/0/on/off/yes/no)
        @param value Standard string value to check for validity
        @return True if value is valid.
    */
    bool isValueValid(const std::string& strValue);

 private:

    /// Item value
    bool m_value;   
};

// ----------------------------------------------------------------------------

/*!
    Class that holds a configuration int32

    -----------------------------------
    |  label: _________________________
    -----------------------------------
              OK    CANCEL
*/

class wizardStepInt32 : public wizardStepBase
{

 public:
    wizardStepInt32();
    ~wizardStepInt32();

    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    void setValue(const std::string& strValue);

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void);

    /*!
        Get the real value from dialog value (can be index into list etc)
        @param input String that should be handled
        @return Real value on standard string form
    */
    std::string getRealValue(const std::string& input) { return getValue(); };

    /*!
        Check if the value is valid
        @param value int32_t value to check for validity
        @return True if value is valid.
    */
    bool isValueValid(const std::string& strValue);

    // Getters/Setters for max value
    int32_t getMaxValue() { return m_max; };
    void setMaxValue(int32_t max) { m_max = max; };

    // Getters/Setters for min value
    int32_t getMinValue() { return m_min; };
    void setMinValue(int32_t min) { m_min = min; };

 private:

    /// Item value
    int32_t m_value;

    /// Item max value
    int32_t m_max;

    /// Item min value
    int32_t m_min;   
};

// ----------------------------------------------------------------------------

/*!
    Class that holds a configuration uint32

    -----------------------------------
    |  label: _________________________
    -----------------------------------
              OK    CANCEL
*/

class wizardStepUInt32 : public wizardStepBase
{

 public:
    wizardStepUInt32();
    ~wizardStepUInt32();

    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    void setValue(const std::string& strValue);

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void);

    /*!
        Get the real value from dialog value (can be index into list etc)
        @param input String that should be handled
        @return Real value on standard string form
    */
    std::string getRealValue(const std::string& input) { return getValue(); };

    /*!
        Check if the value is valid
        @param value uint32_t value to check for validity
        @return True if value is valid.
    */
    bool isValueValid(const std::string& strValue);

    // Getters/Setters for max value
    uint32_t getMaxValue() { return m_max; };
    void setMaxValue(uint32_t max) { m_max = max; };

    // Getters/Setters for min value
    uint32_t getMinValue() { return m_min; };
    void setMinValue(uint32_t min) { m_min = min; };

 private:

    /// Item value
    uint32_t m_value;

    /// Item max value
    uint32_t m_max;

    /// Item min value
    uint32_t m_min;
};

// ----------------------------------------------------------------------------

/*!
    Class that holds a configuration int64

    -----------------------------------
    |  label: _________________________
    -----------------------------------
              OK    CANCEL
*/

class wizardStepInt64 : public wizardStepBase
{

 public:
    wizardStepInt64();
    ~wizardStepInt64();

    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    void setValue(const std::string& strValue);

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void);

    /*!
        Get the real value from dialog value (can be index into list etc)
        @param input String that should be handled
        @return Real value on standard string form
    */
    std::string getRealValue(const std::string& input) { return getValue(); };

    /*!
        Check if the value is valid
        @param value int64_t value to check for validity
        @return True if value is valid.
    */
    bool isValueValid(const std::string& strValue);

    // Getters/Setters for max value
    int64_t getMaxValue() { return m_max; };
    void setMaxValue(int64_t max) { m_max = max; };

    // Getters/Setters for min value
    int64_t getMinValue() { return m_min; };
    void setMinValue(int64_t min) { m_min = min; };

 private:

    /// Item value
    int64_t m_value;

    /// Item max value
    int64_t m_max;

    /// Item min value
    int64_t m_min;  

};

// ----------------------------------------------------------------------------

/*!
    Class that holds a configuration uint64

    -----------------------------------
    |  label: _________________________
    -----------------------------------
              OK    CANCEL
*/
class wizardStepUInt64 : public wizardStepBase
{

 public:
    wizardStepUInt64();
    ~wizardStepUInt64();

    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    void setValue(const std::string& strValue);

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void);

    /*!
        Get the real value from dialog value (can be index into list etc)
        @param input String that should be handled
        @return Real value on standard string form
    */
    std::string getRealValue(const std::string& input) { return getValue(); };

    /*!
        Check if the value is valid
        @param value uint64_t value to check for validity
        @return True if value is valid.
    */
    bool isValueValid(const std::string& strValue);

    // Getters/Setters for max value
    uint64_t getMaxValue() { return m_max; };
    void setMaxValue(uint64_t max) { m_max = max; };

    // Getters/Setters for min value
    uint64_t getMinValue() { return m_min; };
    void setMinValue(uint64_t min) { m_min = min; };

 private:

    /// Item value
    uint64_t m_value;

    /// Item max value
    uint64_t m_max;

    /// Item min value
    uint64_t m_min;

};

// ----------------------------------------------------------------------------

/*!
    Class that holds a configuration float

    -----------------------------------
    |  label: _________________________
    -----------------------------------
              OK    CANCEL
*/

class wizardStepFloat : public wizardStepBase
{

 public:
    wizardStepFloat();
    ~wizardStepFloat();

    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    void setValue(const std::string& strValue);

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void);

    /*!
        Get the real value from dialog value (can be index into list etc)
        @param input String that should be handled
        @return Real value on standard string form
    */
    std::string getRealValue(const std::string& input) { return getValue(); };

    /*!
        Check if the value is valid
        @param value Float value to check for validity
        @return True if value is valid.
    */
    bool isValueValid(const std::string& strValue);

    // Getters/Setters for max value
    double getMaxValue() { return m_max; };
    void setMaxValue(double max) { m_max = max; };

    // Getters/Setters for min value
    double getMinValue() { return m_min; };
    void setMinValue(double min) { m_min = min; };

 private:
    
    /// Item value
    double m_value;
    
    /// Item max value
    double m_max;

    /// Item min value
    double m_min;
};



// ----------------------------------------------------------------------------




// This class represent one wizard choice

class wizardChoiceItem
{
 public:
    wizardChoiceItem();
    ~wizardChoiceItem();

    /*!
        Set one choices item
        @param value Value on string form for choice
        @param description Description of this choice
    */
    void setChoice(const std::string& value, const std::string& description) 
        { m_value = value; m_description = description; };

    /*!
        Set choice value
        @param value Value on string form for choice
    */
    void setValue(const std::string& value) { m_value=value; };
    
    
    /*!
        Get choice value
        @return Value for choice as standard string
    */
    std::string getValue(void) { return m_value; };

    /*!
        Set choice description
        @param description Description of this choice
    */
    void setDescription(const std::string& description) { m_description = description; };

    /*!
        Get choice description
        @return Description for choice as standard string
    */
    std::string getDescription(void) { return m_description; };        

 protected:

    /// Choice value
    std::string m_value;

    /// Choice description
    std::string m_description;

};


/*!
    Class that holds a configuration choice

    -----------------------------------
    |  item 1
    |  item 2
    |  item 3
    |  .....
    |  item 4
    -----------------------------------
              OK    CANCEL
*/
class wizardStepChoice : public wizardStepBase
{

 public:
    wizardStepChoice();
    ~wizardStepChoice();


    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    virtual void setValue(const std::string& strValue) { m_value = strValue; };


    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void) { return m_value; };

    /*!
        Get the real value from dialog value (can be index into list etc)
        @param input String that should be handled
        @return Real value on standard string form
    */
    std::string getRealValue(const std::string& input);

    /*!
        Add choice item
        @param choice Pointer to wizardItemChoice
    */
    void addChoice(wizardChoiceItem *choice) { m_choices.push_back(choice); };

 public:

    /// List with choices
    std::vector<wizardChoiceItem *> m_choices;

 private:

    /// Index for selected item  (0 = default, first item)
    std::string m_value;

    
};



////////////////////////////////////////////////////////////////////////////////
//                             Flag steps
////////////////////////////////////////////////////////////////////////////////



// ----------------------------------------------------------------------------

// This class represent one wizard choice

class wizardFlagBitBase
{

public:    

    wizardStepBase::wizardType m_type = wizardStepBase::wizardType::NONE;    

 public:
    wizardFlagBitBase();
    ~wizardFlagBitBase();

    /*!
        Set value
        @param strValue Value on string format
    */
    virtual void setValue(const std::string& strValue) = 0;

    /*!
        Get value
        @return Value on string form
    */
    virtual std::string getValue(void) = 0;

    /*!
        Get the real value as unsigned integer from dialog flag value 
        (can be index into list etc)
        @param input String that should be handled
        @return Real value as unsigned int
    */
    virtual uint32_t getRealValue(const std::string& input) = 0;

    /*!
        Set position in flags integer (Must be in range 0-31)
        @param pos pos to set 
    */
    void setPos(uint8_t pos) { m_pos = (pos & 0x1f); };

    /*!
        Get set pos
        @return Set pos
    */
    uint8_t getPos(void) { return m_pos; };

    /*!
        Set bitfield width in flags integer (Must be in range 0-31)
        @param pos pos to set 
    */
    void setWidth(uint8_t width) { m_width = (width & 0x1f); };

    /*!
        Get bitfield width in flags integer
        @return Set bitfield width
    */
    uint8_t getWidth(void) { return m_width; };

    /*!
        Set description for bit(s)
        @param description Description for bit(s)
    */
    void setDescription(const std::string description) { m_description = description; };

    /*!
        Get description for bit(s)
        @return Description for bit(s) as standard string
    */
    std::string getDescription(void) { return m_description; };


    /*!
        Set info URL for bit(s)
        @param infourl Info URL for bit(s) 
    */
    void setInfoUrl(const std::string infourl) { m_infourl = infourl; };

    /*!
        Get info URL for bit(s)
        @return Info URL for bit(s)
    */
    std::string getInfoUrl(void) { return m_infourl; };            

 protected:

    /// Position for flag bit 0-31 (deafult = 0)
    uint16_t m_pos;

    /// Width of bit field. 1-32 (default = 1)
    uint8_t m_width;

    /// Description of item
    std::string m_description;

    /// Pointer to URL describing the item
    std::string m_infourl;
};


// ----------------------------------------------------------------------------



/*!
    Class that holds a configuration boolean

    -----------------------------------
    |          x checkbox
    -----------------------------------
              OK    CANCEL
*/

class wizardFlagBitBool : public wizardFlagBitBase
{

 public:
    wizardFlagBitBool();
    ~wizardFlagBitBool();

    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    void setValue(const std::string& strValue);

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void); 

    /*!
        Get the real value as unsigned integer from dialog flag value 
        (can be index into list etc)
        @param input String that should be handled
        @return Real value as unsigned int
    */
    uint32_t getRealValue(const std::string& input);

    /*!
        Check if the value is valid (true/false/1/0/on/off/yes/no)
        @param value bool value to check for validity
        @return True if value is valid.
    */
    bool isValueValid(const std::string& strValue);

 private:

    /// Item value
    bool m_value;   
};



// ----------------------------------------------------------------------------



/*!
    Class that holds a configuration uint32

    -----------------------------------
    |  label: _________________________
    -----------------------------------
              OK    CANCEL
*/

class wizardFlagBitNumber : public wizardFlagBitBase
{

 public:
    wizardFlagBitNumber();
    ~wizardFlagBitNumber();

    /*!
        Set the value form it's string representation
        @param strValue Value on string form
        @return true on success, false on failure.
    */
    void setValue(const std::string& strValue);

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void);

    /*!
        Get the real value as unsigned integer from dialog flag value 
        (can be index into list etc)
        @param input String that should be handled
        @return Real value as unsigned int
    */
    uint32_t getRealValue(const std::string& input);

 private:

    /// Item value
    uint32_t m_value;
};



// ----------------------------------------------------------------------------




class wizardBitChoice
{
 public:
    wizardBitChoice();
    ~wizardBitChoice();

    /*!
        Set description for bit(s)
        @param description Description for bit(s)
    */
    void setDescription(const std::string description) { m_description = description; };

    /*!
        Get description for bit(s)
        @return Description for bit(s) as standard string
    */
    std::string getDescription(void) { return m_description; };

    /*!
        Get set value on string form
        @return Value on string form
    */
    std::string getValue(void) { return m_value; };     

    /*!
        Get set value on string form
        @return Value on string form
    */
    void setValue(const std::string& value) { m_value = value; };

 private:

    /// Bit choice value
    std::string m_value;

    /// Bit choice description
    std::string m_description;
};


/*!
    Class that holds a configuration choice

    A flag choice is a choices that can set a bit field of
    size one to 32 bits to a value. pos (0-31) is the fields starting 
    position. Width is number of bits (1-32).

    value & (2^width - 1) gives valied values for a field
    value << pos move the value to the right pos.

    -----------------------------------
    |  item 1
    |  item 2
    |  item 3
    |  .....
    |  item 4
    -----------------------------------
              OK    CANCEL
*/
class wizardFlagBitChoice : public wizardFlagBitBase
{

 public:
    wizardFlagBitChoice();
    ~wizardFlagBitChoice();

    /*!
        Get selected value on string form
        @return Value on string form
    */
    std::string getValue(void) {return m_value; };

    /*!
        Set value
        @param value Value on string form
    */
    void setValue(const std::string& value) { m_value = value; };

    /*!
        Get the real value as unsigned integer from dialog flag value 
        (can be index into list etc)
        @param input String that should be handled
        @return Real value as unsigned int
    */
    uint32_t getRealValue(const std::string& input);

    /*!
        Get the flags value. 
        @return Flags value to add total flags value
    */
    uint32_t getFlagsValue(void);
    

    /*!
        Add flag bit choice
        @param choice Pointer to a wizardBitChoice
    */
    void addChoice(wizardBitChoice *choice) { m_choices.push_back(choice); };

 public:
    
    /// List with choices
    std::vector<wizardBitChoice *> m_choices;

 private:

    /// Selected value
    std::string m_value;    
    
};

// ----------------------------------------------------------------------------



///////////////////////////////////////////////////////////////////////////////
//                                  Main
///////////////////////////////////////////////////////////////////////////////



// Main class for CANAL configurations- This class holds
// the wizard list that defines the configuration steps.

class canalXmlConfig
{

 public:
    canalXmlConfig();
    ~canalXmlConfig();

    /*!
        Add a wizard step
        @param step Wizard step object to add
        @return true on success
    */
    bool addWizardStep(wizardStepBase *item);

    /*!
        Add a wizard flag step
        @param step Wizard flag  object to add
        @return true on success
    */
    bool addFlagWizardStep(wizardFlagBitBase *item);

    /*!
        Parse the XML config and create the wizard
        structure.
        @param xmlcfg CANAL XML configuration
        @return true on success, false on failure
    */
    bool parseXML(const std::string& xmlcfg);

    // Setters for description
    void setDescription(const std::string& description) { m_description = description; };
    std::string getDescription(void) { return m_description; }

    // Setters for level
    void setLevel(uint8_t level) { m_level = m_level; };
    uint8_t getLevel(void) { return m_level; }

    // Setters for bBlocking
    void setBlocking(bool b) { m_bBlocking = b; };
    bool getBlocking(void) { return m_bBlocking; }

    // Setters for description
    void setInfoUrl(const std::string& infourl) { m_infourl = infourl; };
    std::string getInfoUrl(void) { return m_infourl; }

 public:

    // List with wizard item steps (objects)
    std::list<wizardStepBase *> m_stepsWizardItems;  

    // List with wizard flag steps (objects)
    std::list<wizardFlagBitBase *> m_stepsWizardFlags;

 private:

    /// Description for driver
    std::string m_description;

    /// Driver level VSCP_LEVEL1 or VSCP_LEVEL2
    uint8_t m_level;

    /// Blocking flag
    bool m_bBlocking;

    /// URL to info about driver
    std::string m_infourl;
};



#endif

