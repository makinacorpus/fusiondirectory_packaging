<div id="{$sectionId}"  class="plugin_section">
  <span class="legend">
    {$section}
  </span>
  <div>
    <img src="{$attributes.users_stats.img|escape}" alt="user icon"/>
    {t count=$attributes.users_stats.nb 1=$attributes.users_stats.nb plural="There are %1 users:"}There is 1 user:{/t}
    <ul>
      {foreach from=$attributes.users_stats.accounts item=acc}
        <li style="list-style-image:url({$acc.img})">
        {if $acc.nb > 0}
          {t count=$acc.nb 1=$acc.name 2=$acc.nb plural="%2 of them have a %1 account"}One of them have a %1 account{/t}
        {else}
          {t 1=$acc.name}None of them have a %1 account{/t}
        {/if}
        </li>
      {/foreach}
      <li style="list-style-image:url({$attributes.users_stats.locked_accounts.img})">
        {if $attributes.users_stats.locked_accounts.nb > 0}
          {t count=$attributes.users_stats.locked_accounts.nb 1=$attributes.users_stats.locked_accounts.nb plural="%1 of them are locked"}One of them is locked{/t}
        {else}
          {t}None of them is locked{/t}
        {/if}
      </li>
    </ul>
  </div>
</div>
