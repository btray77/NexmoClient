<?php
/**
 * Nexmo API Client
 * Performs requests to the Nexmo API
 *
 * @link http://www.nexmo.com/documentation/ 
 *
 * @author Dustin Doiron <dustin@weebly.com>
 * @since 2011-11-01
 */
class NexmoClient
{
  /**
   * Defines whether or not Nexmo is enabled
   */
  const NEXMO_API_ENABLED = false;

  /**
   * Nexmo API endpoint
   */
  const API_ENDPOINT_URI = 'http://rest.nexmo.com/sms/json?';

  /**
   * Nexmo API key
   */
  const API_KEY = '';

  /**
   * Nexmo API secret
   */
  const API_SECRET = '';

  /**
   * Default SMS Sender
   * Purchased through Nexmo
   * @link http://dashboard.nexmo.com/private/numbers
   */
  const DEFAULT_SENDER = '';

  /**
   * Nexmo Related Errors
   */
  const ERROR_API_DISABLED = 'API_DISABLED';
  const ERROR_API_FATAL = 'API_ERROR_FATAL';
  const ERROR_NUMBER_BARRED = 'NUMBER_BARRED';
  const ERROR_NUMBER_INVALID = 'NUMBER_INVALID';
  const ERROR_NUMBER_INVALID_PREFIX = 'NUMBER_INVALID_PREFIX';

  /**
   * Informs whether or not the NexmoAPI is currently enabled
   */
  private $isEnabled = NULL;

  /**
   * Constructor
   */
  private function construct( )
  {
    /**
     * Populate the isEnabled member variable
     */
    $this->isEnabled( );
  }

  /**
   * Retrieve an instance of this application
   *
   * @return instance
   */
  public static function getInstance( )
  {
    static $instance = NULL;

    if ( $instance === NULL )
    {
      $instance = new NexmoClient( );
    }

    return $instance;
  }

  /**
   * Perform a request to the Nexmo API
   *
   * @param int $recipient
   * @param string $message
   * @param string $sender optional sender
   *
   * @return array $result
   * @throws Exception
   */
  public function request( $recipient, $message, $sender = false )
  {
    if ( $this->isEnabled( ) === false )
    {
      $result['message'] = self::ERROR_DISABLED;
      $result['success'] = (bool) false;

      return $result;
    }

    $payload = array(
      'username' => self::API_KEY,
      'password' => self::API_SECRET,
      'to' => $recipient,
      'text' => $message,
      'from' => ( empty( $sender ) ) ? self::DEFAULT_SENDER : $sender,
    );

    $handler = curl_init( );

    $url = self::API_ENDPOINT_URI . http_build_query( $payload );

    $options = array(
      CURLOPT_URL => $url,
      CURLOPT_HEADER => false,
      CURLOPT_POST => true,
      CURLOPT_POSTFIELDS => $payload,
      CURLOPT_RETURNTRANSFER => true,
    );

    curl_setopt_array( $handler, $options );

    $result = json_decode( curl_exec( $handler ) );
    $connectionInfo = curl_getinfo( $handler );
    $connectionInfo['errorString'] = curl_error( $handler );

    curl_close( $handler );

    return $this->responseHandler( $result, $connectionInfo );
  }

  /**
   * Helper method which informs whether or not the flag for this service
   * is set to enabled in the database
   *
   * @return bool
   */
  public function isEnabled( )
  {
    if ( $this->isEnabled === NULL )
    {
      /**
       * In another use, this was populated by a memcache key.
       * Changed to use the constant, but left it in so it's still configurable.
       */
      $this->isEnabled = self::NEXMO_API_ENABLED;
    }

    return $this->isEnabled;
  }

  /**
   * Parses and determines success of message based on response from the Nexmo API
   *
   * @param object $messages
   * @param array $connectionInfo
   *
   * @return array $result
   */
  private function responseHandler( $messages, $connectionInfo )
  {
    if ( isset( $messages ) === false )
    {
      // Nexmo is probably down.
      $result['success'] = (bool) false;
      $result['message'] = self::ERROR_API_FATAL;
      return $result;
    }

    foreach ( $messages->messages as $message )
    {
      switch ( $message->status )
      {
        case 0:
          $result['success'] = true;
          return $result;

        case 7:
          $result['success'] = (bool) false;
          $result['message'] = self::ERROR_NUMBER_BARRED;
          return $result;

        case 3:
          $result['success'] = (bool) false;
          $result['message'] = self::ERROR_NUMBER_INVALID;
          return $result;

        case 2:
          $result['success'] = (bool) false;
          $result['message'] = self::ERROR_NUMBER_INVALID;
          return $result;

        case 6:
          $result['success'] = (bool) false;
          $result['message'] = self::ERROR_NUMBER_INVALID_PREFIX;
          return $result;

        default:
          $result['success'] = (bool) false;
          $result['message'] = self::ERROR_API_FATAL;
          return $result;
      }
    }
  }
}