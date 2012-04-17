<?php

/**
 * Represents a chatroom.
 */
class Chatroom {

  /**
   * Format to be used when filtering chat message output.
   */
  protected $format = '';

  /**
   * List of role ids allowed in this chat.
   */
  protected $allowedRoles = array();

  protected $cid = 0;

  protected $uid = 0;

  protected $public = FALSE;

  protected $title = '';

  protected $latestMessage = FALSE;

  protected $onlineUsers = array();

  public function __construct($cid = FALSE) {
    if ($cid) {
      $data = db_query('SELECT * FROM {chatroom} WHERE cid = :cid', array(':cid' => $cid))->fetchObject();
      if ($data) {
        $this->cid = $data->cid;
        $this->public = $data->public;
        $this->title = $data->title;
        $this->format = $data->format ? $data->format : filter_fallback_format();
      }
    }
  }

  public function getOnlineUsers() {
    $info = nodejs_get_content_channel_users('chatroom_' . $this->get('cid'));
    $users = array();
    if (!empty($info['uids'])) {
      $users = user_load_multiple($info['uids']);
    }
    return $users;
  }

  public function set($key, $value) {
    if (isset($this->$key)) {
      $this->$key = $value;
    }
  }

  public function get($key) {
    if (isset($this->$key)) {
      if (in_array($key, array('latestMessage', 'allowedRoles'))) {
        $loadMethod = 'load' . ucfirst($key);
        $this->$key = $this->$loadMethod();
      }
      return $this->$key;
    }
  }

  protected function loadLatestMessage() {
    $messages = $this->loadLatestMessages(1);
    return $messages ? current($messages) : FALSE;
  }

  protected function loadAllowedRoles() {
    $this->allowedRoles = array();
    $roles = db_query('SELECT rid FROM {chatroom_role} WHERE cid = :cid', array(':cid' => $this->cid))->fetchAll();
    foreach ($roles as $role) {
      $this->allowedRoles[$role->rid] = $role->rid;
    }
    return $this->allowedRoles;
  }

  /**
   * Write a message into a chat, and allow any interested modules to react.
   *
   * @param array $message
   * @return boolean
   */
  public function saveMessage($message) {
    drupal_alter('chatroom_msg', $message);
    if (drupal_write_record('chatroom_msg', $message)) {
      $this->latestMessage = (object) $message;
      foreach (module_implements('chat_msg_saved') as $module) {
        $function = $module . '_chat_msg_saved';
        $function($message, $this);
      }
      return $message['cmid'];
    }
    return FALSE;
  }

  /**
   * Check to see if the given user has access to this chat.
   *
   * @param mixed $account
   * @return boolean
   */
  public function userHasAccess($account) {
    if ($this->get('public')) {
      return TRUE;
    }

    if ($allowed_roles = $this->get('allowedRoles')) {
      return (bool) array_intersect_key($allowed_roles, $account->roles);
    }

    $params = array(
      ':cid' => $this->get('cid'),
      ':uid' => $account->uid,
    );
    return (bool) db_query('SELECT cid FROM {chatroom_user} WHERE cid = :cid AND uid = :uid', $params)->fetchField();
  }

  /**
   * Make the given chat private.
   *
   * @return boolean
   */
  public function makePrivate() {
    return db_query('UPDATE {chatroom} SET public = 0 WHERE cid = :cid', array(':cid' => $this->get('cid')));
  }

  /**
   * Remove a user from a private chat.
   *
   * @param mixed $user_to_be_removed
   * @param mixed $user_doing_the_removing
   * @return boolean
   */
  public function removeUser($uid) {
    $params = array(
      ':cid' => $this->get('cid'),
      ':uid' => $uid,
    );
    return db_query('DELETE FROM {chatroom_user} WHERE cid = :cid AND uid = :uid', $params);
  }

  /**
   * Make the given chat public.
   *
   * @return boolean
   */
  public function makePublic() {
    return db_query('UPDATE {chatroom} SET public = 1 WHERE cid = :cid', array(':cid' => $this->get('cid')));
  }

  /**
   * Write a row for each uid for the given chat to allow access.
   *
   * @param mixed $uids
   * @return void
   */
  public function addUser($uid) {
    $data = array('uid' => $uid, 'cid' => $this->get('cid'));
    drupal_write_record('chatroom_user', $data);
  }

  /**
   * Get the message count for a chat.
   */
  public function getMessageCount() {
    $sql = 'SELECT COUNT(cmid) FROM {chatroom_message} WHERE cid = :cid';
    return db_query($sql, array(':cid' => $this->get('cid')))->fetchField();
  }

  public function save() {
    $data = array(
      'title' => $this->title,
      'public' => $this->public,
      'uid' => $this->uid,
      'format' => $this->format,
    );
    if ($this->cid) {
      $data['cid'] = $this->cid;
      drupal_write_record('chatroom', $data, 'cid');
      module_invoke_all('chatroom_update', $this);
    }
    else {
      drupal_write_record('chatroom', $data);
      $this->cid = $data['cid'];
      $this->addUser($this->uid);
      module_invoke_all('chatroom_insert', $this);
    }
  }

  /**
   * Get messages for a given chat.
   *
   * @param $last_cmid
   *   Only load messages with cmids greater than this value.
   * @return array $messages
  */
  public function loadMessages($last_cmid = 0) {
    $sql = "SELECT cm.*, u.name, 0 AS guest_id
            FROM {chatroom_msg} cm
            INNER JOIN {users} u ON u.uid = cm.uid
            WHERE cm.cid = :cid
            AND cm.cmid > :cmid
            ORDER BY cm.cmid ASC";
    $args = array(
      ':cid' => $this->cid,
      ':cmid' => $last_cmid,
    );
    return $this->loadMessagesHelper($sql, $args);
  }

  /**
   * Load the latest messages.
   *
   * @param $limit
   *   Number of messages to load.
   * @return array
   */
  public function loadLatestMessages($limit = 20) {
    if (!preg_match('/^\d+$/', $limit)) {
      return array();
    }
    $sql = "SELECT cm.*, u.name, 0 AS guest_id
            FROM {chatroom_msg} cm
            INNER JOIN {users} u ON u.uid = cm.uid
            WHERE cm.cid = :cid
            ORDER BY cm.cmid DESC
            LIMIT $limit";
    $messages = $this->loadMessagesHelper($sql, array(':cid' => $this->cid));
    return array_reverse($messages, TRUE);
  }

  protected function loadMessagesHelper($sql, $args) {
    $messages = array();
    $guest_sids = array();
    $guest_prefix = variable_get('chatroom_guest_user_prefix', t('guest-'));
    $allow_anon_names = variable_get('chatroom_allow_anon_name', TRUE);

    $result = db_query($sql, $args)->fetchAll(PDO::FETCH_OBJ);
    foreach ($result as $message) {
      if ($message->uid == 0 && !in_array($message->sid, $guest_sids)) {
        $guest_sids[] = $message->sid;
      }
      $messages[$message->cmid] = $message;
    }

    if (!empty($guest_sids)) {
      // TODO: past beejeebus, please document this again, mkay?
      $result = db_select('chatroom_online_list')
        ->fields('sid')
        ->fields('guest_id')
        ->condition('sid', $guest_sids, 'IN')
        ->execute()
        ->fetchAll(PDO::FETCH_OBJ);
      foreach ($result as $guest) {
        foreach ($messages as $message) {
          if ($message->sid == $guest->sid) {
            if ($message->anon_name && $allow_anon_names) {
              $messages[$message->cmid]->name = $message->anon_name;
              $messages[$message->cmid]->guest_prefix = $guest_prefix;
            }
            else {
              $messages[$message->cmid]->name = $guest_prefix . $guest->guest_id;
            }
            $messages[$message->cmid]->guest_id = $guest->guest_id;
          }
        }
      }
    }
    return $messages;
  }

  /**
   * Update the last seen time for a the given session id in the given chat.
   *
   * @param mixed $session_id
   * @return void
   */
  function updateLastSeenTime($session_id) {
    $sql = "UPDATE {chatroom_online_list}
            SET last_seen_time = :last_seen_time
            WHERE ccid = :ccid AND sid = :sid";
    db_query($sql, array(':last_seen_time' => time(), ':cid' => $this->cid, ':sid' => $session_id));
  }
}

