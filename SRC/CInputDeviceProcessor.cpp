#include "CInputDeviceProcessor.hpp"

// METHODS
/** Constructor.
 * Objects of this class can only be instantiated and destroyed by derived classes, thus making this class abstract. */
CInputDeviceProcessor::CInputDeviceProcessor( void )
{
}


/** Constructor.
 * Objects of this class can only be instantiated and destroyed by derived classes, thus making this class abstract. */
CInputDeviceProcessor::CInputDeviceProcessor( const CInputDeviceProcessor & )
{
}


/** Constructor.
 * Objects of this class can only be instantiated and destroyed by derived classes, thus making this class abstract. */
CInputDeviceProcessor::~CInputDeviceProcessor( void )
{
}


/** Called when the user selects a new Clipping Algorithm in the application, to warn the Input Processors.
 * @param clippingAlgorithmId The identifier for the new clipping algorithm that has been selected by the user. */
void CInputDeviceProcessor::OnClippingAlgorithmChanged( int clippingAlgorithmId )
{
}



/** Called when the Input Device's Connection state has chanded.
 * @param bConnected A flag indicating if the device's connection has been established (true) or lost (false). */
void CInputDeviceProcessor::OnInputDeviceConnectionChanged( bool bConnected )
{
}


/** Called when the Input Device's state has been updated. */
void CInputDeviceProcessor::OnInputDeviceStateUpdated( void )
{
}
